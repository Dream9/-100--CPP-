#ifndef _SOLUTION_OTSU_H_
#define _SOLUTION_OTSU_H_

#include"Solution/solution.h"

namespace digital {

//brief;根据类间最大方差法自动确定二值化的阈值
class Otsu : public Solution {
public:
	Otsu(const string& path, bool flag, const string& name = "Otsu")
		:Solution(name, path, flag) 
	{
		getDescriptionHandler().assign("使用大津二值化算法（类间最大方差算法）进行灰度处理");
	}
	~Otsu() override {}

	void operator()() override;

private:
	double getOtsuThreshold_(const void* img);
};

}//！namespace digital

#endif