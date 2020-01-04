#ifndef _SOLUTION_CONNECT_NUMBER_H_
#define _SOLUTION_CONNECT_NUMBER_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class ConnectNumber : public Solution {
public:
	//brief:邻接方式
	enum {
		LINE_4 = 4,
		LINE_8 = 8
	};

	ConnectNumber(const string& path, int op, bool flag = false, const string& name = "ConnectNumber")
		:Solution(name, path, flag), line_type_(op)
	{
		getDescriptionHandler().assign("计算图像中点的连通个数");
	}
	~ConnectNumber() override { }

	void operator()()override;

private:
	int line_type_;
};
}


#endif // !_SOLUTION_CONNECT_NUMBER_H_
