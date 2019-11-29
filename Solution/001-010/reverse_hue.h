#ifndef _SOLUTION_REVERSE_HUE_
#define _SOLUTION_REVERSE_HUE_
#include"Solution/solution.h"

namespace digital {

//brief：调整H分量实现反转
class ReverseHue :public Solution {
public:
	ReverseHue(const string& path, bool flag = false, const string& name = "RerverseHue")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("将色相反转输出");
	}

	~ReverseHue()override {  }

	void operator()()override;

private:
	void rgb2hsi_(uint8_t r, uint8_t g, uint8_t b, double& h, double& s, double& i);
	void hsi2rgb_(double h, double s, double i, uint8_t& r, uint8_t& g, uint8_t& b);
};

}//!namespace digital

#endif