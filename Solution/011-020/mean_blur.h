#ifndef _SOLUTION_MEAN_BLUR_H_
#define _SOLUTION_MEAN_BLUR_H_

#include"Solution/solution.h"

namespace digital {

class MeanBlur :public Solution {
public:
	MeanBlur(const string& path, int win, bool flag = false, const string& name = "MeanBlur")
		:Solution(name,path,flag),win_(win)
	{
		assert(win_ > 0);
		getDescriptionHandler().assign("ʹ�þ�ֵ�˲�����ͼ��");
	}
	~MeanBlur() override {}

	void operator()()override;

private:
	int win_;

};
}



#endif