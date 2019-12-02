#ifndef _SOLUTION_SOBEL_OPERATOR_H_
#define _SOLUTION_SOBEL_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:sobel����,��������������Ȩ�ص�һ��΢������
class SobelOperator : public Solution {
public:
	SobelOperator(const string& path, bool flag = false, const string& name = "SobelOperator")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("ͨ��Sobel���Ӽ��ˮƽ/��ֱ���أ�������ݶ�ͼ��");
	}
	~SobelOperator()override {}

	void operator()()override;

private:
	static const int win_;
};
}
#endif // !_SOLUTION_SOBEL_OPERATOR_H_
