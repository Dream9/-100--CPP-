#ifndef _SOLUTION_GAMMA_CORRECTION_H_
#define _SOLUTION_GAMMA_CORRECTION_H_

#include"Solution/solution.h"

namespace digital {

//brief:Τ��-��ϣ�ɶ����Ǳ�����������������֮���ϵ�Ķ��ɣ����о��Ĳ��������ԭ���̼����ı仯���仯��
//     ���ұ���Ϊһ���Ĺ����ԣ��ù�ʽ����ʾ�����ǡ���/��=C�����Ц�Ϊԭ�̼���������Ϊ��ʱ�Ĳ�����ޣ�
//     CΪ�������ֳ�ΪΤ����
//     ٤��任��Ŀ�ľ���Τ�����������
class GammaCorrection : public Solution {
public:
	GammaCorrection(const string& path, 
		double c = 1.5,
		double gamma = 2.5 ,
		bool flag = false,
		const string& name = "GammaCorrection")
		:Solution(name, path, flag),
		c_(c),
		gamma_(gamma)
	{
		getDescriptionHandler().assign("��ϵ����豸��������Ӧ");
	}
	~GammaCorrection()override {}

	void operator()()override;

private:
	double  c_;
	double gamma_;
};

}//!namespace digital


#endif // !_SOLUTION_GAMMA_CORRECTION_H_
