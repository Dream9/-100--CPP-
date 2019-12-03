#ifndef _SOLUTION_PREWITT_OPERATOR_H_
#define _SOLUTION_PREWITT_OPERATOR_H_

#include"Solution/solution.h"

namespace digital {

//brief:Prewitt ���Ӽ���һ��΢�֣���û�н��м�Ȩ
class PrewittOperator : public Solution{
public:
	PrewittOperator(const string& path, bool flag = false, const string& name = "PrewittOperator")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("ʹ��Prewitt���Ӽ����ݶ�ͼ��");
	}
	~PrewittOperator() override{}

	void operator()() override;
};

}


#endif // !_SOLUTION_PREWITT_OPERATOR_H_