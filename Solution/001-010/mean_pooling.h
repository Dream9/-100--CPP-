#ifndef _SOLUTION_MEAN_POOLING_H_
#define _SOLUTION_MEAN_POOLING_H_ 

#include"Solution/solution.h"

namespace digital {

//brief:
class MeanPooling : public Solution {
public:
	MeanPooling(const string& path, int win = 8, bool flag = false, const string& name ="MeanPooling")\
		: Solution(name, path, flag), win_(win)
	{
		assert(win_ >= 1);
		getDescriptionHandler().assign("��ͼ����о�ֵ�ػ�(Ĭ��ʹ��8*8����)");
	}

	~MeanPooling()override {  }

	void operator()()override;

private:
	int win_;
};

}


#endif