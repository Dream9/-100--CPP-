#ifndef _SOLUTION_NORMALIZATION_H_
#define _SOLUTION_NORMALIZATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:针对单一像素灰度值的操作，可以拉伸动态范围
class Normalization : public Solution {
public:
	Normalization(const string& path, bool show_pdf = false, bool flag = false, const string& name = "Normalization")
		:Solution(name, path, flag) ,show_pdf_(show_pdf)
	{
		getDescriptionHandler().assign("通过直方图正规化，使其动态范围覆盖整个区间");
	}
	~Normalization()override {}

	void operator()()override;
private:
	bool show_pdf_;
};
}

#endif // !_SOLUTION_NORMALIZATION_H_
