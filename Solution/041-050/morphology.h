#ifndef _SOLUTION_MORPHOLOGY_DILATE_H_
#define _SOLUTION_MORPHOLOGY_DILATE_H_

#include"Solution/solution.h"

namespace digital {

//brief:��̬ѧ�任
//becare:��047-053��������Ŀ�Ĳ����������ڴ˴����
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
		getDescriptionHandler().assign("��ͼ�������̬ѧ���ʹ���");
	}
	~Morphology()override {}

	void operator()()override;

private:
	int iter_;
	int op_;

};

}//!namesapce digital

#endif // !_SOLUTION_MORPHOLOGY_DILATE_H_
