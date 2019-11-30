#ifndef _SOLUTION_DIFFERENTIAL_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:使用最简单的dy/df 和 dx/df表征图像的灰度变换
class DifferentialFilter :public Solution {
public:
	DifferentialFilter(const string& path, bool flag = true, const string& name = "DifferentialFilger")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("使用一阶微分提取图像边界");
	}
	~DifferentialFilter()override {}

	void operator()()override;

private:
	static const int kWin;
};
}

#endif // !_SOLUTION_DIFFERENTIAL_FILTER_H_
