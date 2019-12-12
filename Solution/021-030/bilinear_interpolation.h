#ifndef _SOLUTION_BILINEAR_INTERPOLATION_H_
#define _SOLUTION_BILINEAR_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:双线性内插的方式比最近邻产生的结果要平滑，失真较少
class BilinearInterpolation : public Solution {
public:
	BilinearInterpolation(const string& path, bool flag = false, const string& name = "BilinearInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("采用双线性内插的采样方式调整变换后的图像");
	}
	~BilinearInterpolation()override {}

	void operator()()override ;
};

}


#endif // !_SOLUTION_BILINEAR_INTERPOLATION_H_
