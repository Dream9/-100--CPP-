#ifndef _SOLUTION_CONNECTED_COMPONENT_LABEL_H_
#define _SOLUTION_CONNECTED_COMPONENT_LABEL_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class ConnectedComponentLabel : public Solution {
public:
	enum {
		LINE_4 = 4,
		LINE_8 = 8
	};
	ConnectedComponentLabel(const string& path, int op = LINE_4, bool flag = false, const string& name = "ConnectedComponentLabel")
		:Solution(name, path, flag), line_type_(op)
	{
		getDescriptionHandler().assign("按照不同的邻域规则，标注出不同的连通分量");
	}
	~ConnectedComponentLabel() override {};

	void operator()()override;

private:
	int line_type_;
};
}

#endif // !_SOLUTION_CONNECTED_COMPONENT_LABEL_H_
