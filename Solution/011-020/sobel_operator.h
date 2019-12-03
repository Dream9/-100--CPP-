#ifndef _SOLUTION_SOBEL_OPERATOR_H_
#define _SOLUTION_SOBEL_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:sobel����,��������������Ȩ�ص�һ��΢������
class SobelOperator : public Solution {
public:
	//becare��win����λ���������Ҳ�����11,���С�ڵ���1���Զ�ȡ3
	SobelOperator(const string& path, int win, bool flag = false, const string& name = "SobelOperator")
		:Solution(name, path, flag), win_(win <= 1 ? 3 : win)
	{
		assert((win_ & 0x1) == 0x1);
		assert(win_ <= 11);

		getDescriptionHandler().assign("ͨ��Sobel���Ӽ��ˮƽ/��ֱ���أ�������ݶ�ͼ��");
	}
	~SobelOperator()override {}

	void operator()()override;

private:
	//���ڴ�С����ͬ������sobel����
	const int win_;
};
}
#endif // !_SOLUTION_SOBEL_OPERATOR_H_
