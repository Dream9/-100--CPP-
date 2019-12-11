#ifndef _SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_
#define _SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:图像几何变换分为两个过程：确定位置以及重采样，本类测试了以最近邻方式重采样图像
class NearestNeighborInterpolation : public Solution {
public:
	NearestNeighborInterpolation(const string& path, bool flag = false, const string& name = "NearestNeighborInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("采用最近邻插值的方式缩放图像");
	}
	~NearestNeighborInterpolation()override {}

	void operator()()override;
};

}
#endif // !_SOLUTION_NEAREST_NEIGHBOR_INTERPOLATION_H_
