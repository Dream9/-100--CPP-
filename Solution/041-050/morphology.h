#ifndef _SOLUTION_MORPHOLOGY_DILATE_H_
#define _SOLUTION_MORPHOLOGY_DILATE_H_

#include"Solution/solution.h"

namespace digital {

//brief:形态学变换
//becare:从047-053的所有题目的测试用例都在此处完成
class Morphology: public Solution {
public:
	enum {
		DILATE = 0x1,
		ERODE = 0x1<<1,

		OPEN = 0x1<<2,
		CLOSE = 0x1<<3,

	};
	Morphology(const string& path, int iterations = 1, int op = DILATE, bool flag = false, const string& name = "MorphologyDilate")
		:Solution(name, path, flag), iter_(iterations), op_(op)
	{
		assert(iter_ >= 0);
		getDescriptionHandler().assign("对图像进行形态学膨胀处理");
	}
	~Morphology()override {}

	void operator()()override;

private:
	int iter_;
	int op_;

};

}//!namesapce digital

#endif // !_SOLUTION_MORPHOLOGY_DILATE_H_
