#ifndef _SOLUTION_REDUCE_COLOR_H_
#define _SOLUTION_REDUCE_COLOR_H_

#include"Solution/solution.h"

namespace digital {

class ReduceColor :public Solution {
public:
	ReduceColor(const string&path, bool flag = false, const string& name = "ReduceColor")
		:Solution(name, path, flag) 
	{
		getDescriptionHandler().assign("�����Ϊ8λ��ͼ����м�ɫ�������͵�4^3");
	}

	~ReduceColor()override {  }

	void operator()()override;
};
}



#endif