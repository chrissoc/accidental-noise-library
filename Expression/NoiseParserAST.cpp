#include "NoiseParserAST.h"

#include <math.h>
#include <sstream>

namespace anl
{
	namespace NoiseParserAST
	{
		void argumentList::ShapeUpRecurse(std::vector<NodePtr>& ChilderenTop)
		{
			if(Child.size() >= 1)
				ChilderenTop.push_back(Child[0]);
			if (Child.size() == 2)
				dynamic_cast<argumentList*>(Child[1].get())->ShapeUpRecurse(ChilderenTop);
		}

		void argumentList::ShapeUp()
		{
			if (Child.size() == 2)
			{
				// skip ourself and start at the child.
				dynamic_cast<argumentList*>(Child[1].get())->ShapeUpRecurse(Child);
				// and remove the argumentList Child, leaving just the arguments
				Child.erase(Child.begin() + 1);
			}
		}
	}
}