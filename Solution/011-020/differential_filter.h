#ifndef _SOLUTION_DIFFERENTIAL_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:ʹ����򵥵�dy/df �� dx/df����ͼ��ĻҶȱ任
class DifferentialFilter :public Solution {
public:
	DifferentialFilter(const string& path, bool flag = true, const string& name = "DifferentialFilger")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("ʹ��һ��΢����ȡͼ��߽�");
	}
	~DifferentialFilter()override {}

	void operator()()override;

private:
	static const int kWin;
};
}

#endif // !_SOLUTION_DIFFERENTIAL_FILTER_H_
