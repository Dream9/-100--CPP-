#ifndef _SOLUTION_NORMALIZATION_H_
#define _SOLUTION_NORMALIZATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:��Ե�һ���ػҶ�ֵ�Ĳ������������춯̬��Χ
class Normalization : public Solution {
public:
	Normalization(const string& path, bool show_pdf = false, bool flag = false, const string& name = "Normalization")
		:Solution(name, path, flag) ,show_pdf_(show_pdf)
	{
		getDescriptionHandler().assign("ͨ��ֱ��ͼ���滯��ʹ�䶯̬��Χ������������");
	}
	~Normalization()override {}

	void operator()()override;
private:
	bool show_pdf_;
};
}

#endif // !_SOLUTION_NORMALIZATION_H_
