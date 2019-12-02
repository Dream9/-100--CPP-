#ifndef _SOLUTION_SOBEL_OPERATOR_H_
#define _SOLUTION_SOBEL_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:sobel算子,对中心像素增加权重的一阶微分算子
class SobelOperator : public Solution {
public:
	SobelOperator(const string& path, bool flag = false, const string& name = "SobelOperator")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("通过Sobel算子检测水平/垂直边沿，并获得梯度图像");
	}
	~SobelOperator()override {}

	void operator()()override;

private:
	static const int win_;
};
}
#endif // !_SOLUTION_SOBEL_OPERATOR_H_
