#ifndef _SOLUTION_OTSU_H_
#define _SOLUTION_OTSU_H_

#include"Solution/solution.h"

namespace digital {

//brief;���������󷽲�Զ�ȷ����ֵ������ֵ
class Otsu : public Solution {
public:
	Otsu(const string& path, bool flag, const string& name = "Otsu")
		:Solution(name, path, flag) 
	{
		getDescriptionHandler().assign("ʹ�ô���ֵ���㷨�������󷽲��㷨�����лҶȴ���");
	}
	~Otsu() override {}

	void operator()() override;

private:
	double getOtsuThreshold_(const void* img);
};

}//��namespace digital

#endif