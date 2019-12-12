#ifndef _SOLUTION_BICUBIC_INTERPOLATION_H_
#define _SOLUTION_BICUBIC_INTERPOLATION_H_

#include"Solution/solution.h"

namespace digital {

//brief:˫���β�ֵ�ǶԾ������޲�����������ıƽ�������ز��������ʧ����
//     sinc(x) = sin(PI*x)/x,����Ƶ�����Ӧ���޴����ͨ�˲��������Ͽ��������ֵ
class BicubicInterpolation : public Solution {
public:
	BicubicInterpolation(const string& path, bool flag = false, const string& name = "BicubicInterpolation")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("����˫���β�ֵ�ķ�ʽ�����ز���");
	}

	~BicubicInterpolation() override {}

	void operator()()override;
};

}//!namespace digital



#endif // !_SOLUTION_BICUBIC_INTERPOLATION_H_
