#ifndef _SOLUTION_BICUBIC_INTERPOLATION_H_
#define _SOLUTION_BICUBIC_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:双三次插值是对具有无限波瓣的辛格函数的逼近，因此重采样结果损失更低
//     sinc(x) = sin(PI*x)/x,他在频率域对应有限带宽低通滤波，理论上可以无损插值
class BicubicInterpolation : public Solution {
public:
	BicubicInterpolation(const string& path, bool flag = false, const string& name = "BicubicInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("采用双三次插值的方式进行重采样");
	}

	~BicubicInterpolation() override {}

	void operator()()override;
};

}//!namespace digital



#endif // !_SOLUTION_BICUBIC_INTERPOLATION_H_
