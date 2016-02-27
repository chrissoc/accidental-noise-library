
#pragma once

#include<string>
#include<vector>
#include<stack>
#include <memory>

#include "../VM/vm.h"
#include "../VM/kernel.h"
#include "../VM/instruction.h"

#include "NoiseParserToken.h"
#include "NoiseParserEmitter.h"

/*
whiteSpace ::= ' ' | '\n' | '\r' | '\t'
printableChar ::= whiteSpace | [!-~]
commentLine ::= '/' '/' printableChar*  '\n'
commentBlock ::= '/' '*' printableChar* '*' '/'

domainOp ::= 's' | 't' | 'r'
component ::= 'x' | 'y' | 'z' | 'w' | 'u' | 'v'
letter ::= [a-zA-Z]
digit ::= [0-9]

keyword ::= letter+
number ::= digit+ ('.' digit+)?
domainModifierLeft ::= '<' domainOp component? ':'
domainModifierRight ::= '>'

domainOperator ::= domainModifierLeft argumentList domainModifierRight
argumentList ::= expression (',' argumentList)*
functionCall ::= keyword ( '(' argumentList* ')' )?
object ::= functionCall | grouping | negative | number | keyword
domainPrecedence ::= (domainOperator domainPrecedence) | object
mult ::= domainPrecedence (('*' | '/') mult)?
add ::= mult (('+' | '-') add)?

grouping ::= '(' expression ')'
negative ::= '-' object
expression ::= add | negative | grouping
statement ::= expression ';'
assignment ::= (keyword '=')? statement
program ::= assignment program?
*/

namespace anl
{
	namespace NoiseParserAST
	{
		// Type information was embedded to avoid 
		class Node : public std::enable_shared_from_this<Node> {
		public:
			typedef std::shared_ptr<Node> NodePtr;
			enum Type {
				NUMBER,
				KEYWORD,
				DOMAIN_OPERATOR,
				ARGUMENT_LIST,
				FUNCTION_CALL,
				OBJECT,
				DOMAIN_PRECEDENCE,
				MULT,
				ADD,
				GROUPING,
				NEGATIVE,
				EXPRESSION,
				STATEMENT,
				ASSIGNMENT,
				PROGRAM,
			};
			Type type;
			std::vector<NodePtr> Child;
			Token token;
		public:
			Node(Type t) : type(t) {}
			bool IsType(Type t) { return type == t; }
			virtual void Emit(Emitter* emitter) = 0;
			// Instructs the node to perform any operations needed to come into a state
			// that is considered normative.
			// ie arguemntList with 5 arguments is expected to store all 5 arguments in Child[0-4],
			// but it is originally generated in a recursive state, such that Child[0] is an argument
			// and Child[1] is another argumentList, ShapeUp will convert from the former to the later.
			virtual void ShapeUp() {};
			// Starting at a root node, recursivly call RemoveIntermidiates().
			// This method will return a NodePtr indicating the new node that
			// should be used in its place (if itself was removed)
			virtual NodePtr RemoveIntermidiates()
			{
				for (auto& c : Child)
					if(c) // nullptr is allowed in the child array.
						c = c->RemoveIntermidiates();
				return shared_from_this();
			}

		public:
			// recurse on tree calling ShapeUp on all elements
			void ShapeUpAll()
			{
				ShapeUp();// first ourself.
				// then our children
				for (auto& c : Child)
					if(c) // nullptr is allowd in the ChildArray
						c->ShapeUpAll();
			}
		};

		class number : public Node {
			static const Type MyType = NUMBER;
		public:
			number(const Token& t) : Node(MyType)
			{
				token = t;
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class keyword : public Node {
			static const Type MyType = KEYWORD;
		public:
			keyword(const Token& t) : Node(MyType)
			{
				token = t;
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class domainOperator : public Node {
			static const Type MyType = DOMAIN_OPERATOR;
		public:
			domainOperator(const Token& t, NodePtr argList) : Node(MyType)
			{
				token = t;
				Child.push_back(argList);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class argumentList : public Node {
			static const Type MyType = ARGUMENT_LIST;
		public:
			argumentList(NodePtr arg, NodePtr argList) : Node(MyType)
			{
				if(arg)
					Child.push_back(arg);
				if(argList)
					Child.push_back(argList);
			}
			void ShapeUpRecurse(std::vector<NodePtr>& ChilderenTop);
			void ShapeUp() override;
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class functionCall : public Node {
			static const Type MyType = FUNCTION_CALL;
		public:
			functionCall(NodePtr in_keyword, NodePtr argList) : Node(MyType)
			{
				Child.push_back(in_keyword);
				Child.push_back(argList);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class object : public Node {
			static const Type MyType = OBJECT;
		public:
			object(NodePtr object) : Node(MyType)
			{
				Child.push_back(object);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
			NodePtr RemoveIntermidiates()
			{
				// was just an object, so we are unneeded
				return Child[0]->RemoveIntermidiates();
			}
		};
		class domainPrecedence : public Node {
			static const Type MyType = DOMAIN_PRECEDENCE;
		public:
			// expects either in_operator or in_domainOperator and in_domainPrecedence to be null.
			domainPrecedence(NodePtr in_domainOperator, NodePtr in_domainPrecedence, NodePtr in_object) : Node(MyType)
			{
				if (in_object)
					Child.push_back(in_object);
				else
				{
					Child.push_back(in_domainOperator);
					Child.push_back(in_domainPrecedence);
				}
			}

			void Emit(Emitter* emitter) override { emitter->Emit(this); }

			NodePtr RemoveIntermidiates()
			{
				if (Child.size() == 2)
				{
					Child[0]->RemoveIntermidiates();
					Child[1]->RemoveIntermidiates();
					return shared_from_this();
				}
				else
				{
					// was just an object, so ourself, domainPrecedence, was unneeded
					return Child[0]->RemoveIntermidiates();
				}
			}
		};
		class mult : public Node {
			static const Type MyType = MULT;
		public:
			mult(const Token& t, NodePtr in_left_domainPrecedence, NodePtr in_right_mult) : Node(MyType)
			{
				token = t;
				Child.push_back(in_left_domainPrecedence);
				Child.push_back(in_right_mult);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class add : public Node {
			static const Type MyType = ADD;
		public:
			add(const Token& t, NodePtr in_left_mult, NodePtr in_right_add) : Node(MyType)
			{
				token = t;
				Child.push_back(in_left_mult);
				Child.push_back(in_right_add);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
		class grouping : public Node {
			static const Type MyType = GROUPING;
		public:
			grouping(NodePtr in_expression) : Node(MyType)
			{
				Child.push_back(in_expression);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
			NodePtr RemoveIntermidiates()
			{
				// grouping does not emit anything usefull, so remove ourself.
				return Child[0]->RemoveIntermidiates();
			}
		};
		class negative : public Node {
			static const Type MyType = NEGATIVE;
		public:
			negative(const Token& t, NodePtr in_object) : Node(MyType)
			{
				token = t;
				Child.push_back(in_object);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
			// no obvious intermidates to remove
			//NodePtr RemoveIntermidiates()
		};
		class expression : public Node {
			static const Type MyType = EXPRESSION;
		public:
			expression(NodePtr in) : Node(MyType)
			{
				Child.push_back(in);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }

			NodePtr RemoveIntermidiates()
			{
				// expression does not emit anything usefull, so remove ourself.
				return Child[0]->RemoveIntermidiates();
			}
		};
		class statement : public Node {
			static const Type MyType = STATEMENT;
		public:
			statement(NodePtr in_expression) : Node(MyType)
			{
				Child.push_back(in_expression);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }

			NodePtr RemoveIntermidiates()
			{
				// statement does not emit anything usefull, so remove ourself.
				return Child[0]->RemoveIntermidiates();
			}
		};
		class assignment : public Node {
			static const Type MyType = ASSIGNMENT;
		public:
			// in_keyword is optional, in_statment is optional,  if not there pass a null
			// atleast one must not be null
			assignment(const Token& t, NodePtr in_keyword, NodePtr in_statment) : Node(MyType)
			{
				// the token received here is only the general token related to somthing
				// near the assignment operator.
				token = t;
				Child.push_back(in_keyword);
				Child.push_back(in_statment);
			}
			void Emit(Emitter* emitter) override { emitter->Emit(this); }

			NodePtr RemoveIntermidiates()
			{
				Child[1] = Child[1]->RemoveIntermidiates();
				// if there is nothing to assign to then we dont need
				// to keep this assignment node, just pass our one child instead.
				if (Child[0] == false)
				{
					return Child[1];
				}
				else
				{
					Child[0] = Child[0]->RemoveIntermidiates();
					return shared_from_this();
				}
			}
		};
		class program : public Node {
			static const Type MyType = PROGRAM;
		public:
			program(NodePtr in_assignment, NodePtr in_program) : Node(MyType)
			{
				Child.push_back(in_assignment);
				if(in_program)
					Child.push_back(in_program);
			}
			// program is a special case for Emit() since by design we only
			// care about the last statement in the file we skip all previous
			// statements and start at the last one.
			void Emit(Emitter* emitter) override { emitter->Emit(this); }
		};
	}
}