#ifndef _SOLUTION_REDUCE_COLOR_H_
#define _SOLUTION_REDUCE_COLOR_H_

#include"Solution/solution.h"

namespace digital {

class ReduceColor :public Solution {
public:
	ReduceColor(const string&path, bool flag = false, const string& name = "ReduceColor")
		:Solution(name, path, flag) 
	{
		getDescriptionHandler().assign("对深度为8位的图像进行减色处理，降低到4^3");
	}

	~ReduceColor()override {  }

	void operator()()override;
};
}



#endif