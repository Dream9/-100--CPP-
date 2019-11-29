#ifndef _SOLUTION_MOTION_BLUR_H_
#define _SOLUTION_MOTION_BLUR_H_

#include"Solution/solution.h"

namespace digital {

//brief:运动模糊
class MotionBlur :public Solution {
public:
	MotionBlur(const string& path, int win = 3, double angle = 0, bool flag = false, const string& name = "MotionFilter")
		:Solution(name, path, flag), win_(win), angle_(angle)
	{
		assert(win_ > 0);
		assert((win_ & 0x1) == 0x1);
		getDescriptionHandler().assign("对图像进行运动模糊");
	}
	~MotionBlur() override {};

	void operator()()override;

private:
	void getMotionBlurCoefficient_(int* dst);

	int win_;
	double angle_;

};

}//!namespace digital


#endif // !_SOLUTION_MOTION_BLUR_H_
