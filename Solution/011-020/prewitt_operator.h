#ifndef _SOLUTION_PREWITT_OPERATOR_H_
#define _SOLUTION_PREWITT_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:Prewitt 算子计算一阶微分，并没有进行加权
class PrewittOperator : public Solution{
public:

	PrewittOperator(const string& path, bool flag = false, const string& name = "PrewittOperator")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("使用Prewitt算子计算梯度图像");
	}
	~PrewittOperator() override{}

	void operator()() override;

private:
	void prewitt_(void* src, void* dst, int ddepth, int dx, int dy, int bordertype = BORDER_DEFAULT);
};

}


#endif // !_SOLUTION_PREWITT_OPERATOR_H_
