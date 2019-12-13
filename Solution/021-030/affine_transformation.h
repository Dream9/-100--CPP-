#ifndef _SOLUTION_AFFINE_TRANSFORMATION_H_
#define _SOLUTION_AFFINE_TRANSFORMATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:默认采用双三次插值的方式进行图像的变换
//becare:由于NO_028,NO_029,NO_030,NO_031的题目都是对warpAffine的不同情况的测试，因此统一在此完成
class AffineTransformation : public Solution {
public:
	//标识进行的操作
	enum {
		TRANSLATION = 0x1,  //平移
		SCALE = 0x1 << 1,   //放缩
		ROTATION = 0x1 << 2,//旋转
		LEAN = 0x1 << 3,    //倾斜
	};

	//parameter: op指定测试的操作
	AffineTransformation(const string& path, int op, bool flag = false, const string& name = "AffineTransformation")
		:Solution(name, path, flag),ops_(op)
	{
		getDescriptionHandler().assign("对图像进行如下仿射变换：平移，缩放，旋转");
	}

	~AffineTransformation()override {}

	void operator()()override;

private:

	int ops_;
};

}

#endif // !_SOLUTION_AFFINE_TRANSFORMATION_H_
