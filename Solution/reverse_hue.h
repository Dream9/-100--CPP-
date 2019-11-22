#ifndef _SOLUTION_REVERSE_HUE_
#define _SOLUTION_REVERSE_HUE_
#include"Solution/solution.h"

namespace digital {

//brief������H����ʵ�ַ�ת
class ReverseHue :public Solution {
public:
	ReverseHue(const string& path, bool flag = false, const string& name = "RerverseHue")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("��ɫ�෴ת���");
	}

	~ReverseHue()override {  }

	void operator()()override;

private:
	void rgb2hsi_(uint8_t r, uint8_t g, uint8_t b, double& h, double& s, double& i);
	void hsi2rgb_(double h, double s, double i, uint8_t& r, uint8_t& g, uint8_t& b);
};

}//!namespace digital

#endif