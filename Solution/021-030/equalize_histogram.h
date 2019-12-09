#ifndef _SOLUTION_EQUALIZE_HISTOGRAM_H_
#define _SOLUTION_EQUALIZE_HISTOGRAM_H_

#include"Solution/solution.h"

namespace digital {

//brief:实现直方图均衡化
//     对于给定的图像，均衡化的结果是唯一的，这也是直方图匹配的理论基础
class EqualizeHistogram : public Solution {
public:
	EqualizeHistogram(const string& path, bool show_pdf = false, bool flag = false, const string& name = "EqualizeHistogram")
		:Solution(name, path, flag), show_pdf_(show_pdf)
	{
		getDescriptionHandler().assign("通过直方图均衡化扩大图像动态范围，丰富其灰度细节");
	}
	~EqualizeHistogram()override {}

	void operator()() override;

private:
	bool show_pdf_;

};

}//!namespace digital

#endif // !_SOLUTION_EQUALIZE_HISTOGRAM_H_
