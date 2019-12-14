#ifndef _SOLUTION_FOURIER_H_
#define _SOLUTION_FOURIER_H_

#include"Solution/solution.h"

namespace digital {

//brief:
class Fourier : public Solution {
public:
	Fourier(const string& path, bool flag = false, const string& name = "Fourier")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("进行fourier变换，得到其频谱图，相位图，然后"
			"进行反变换得到原始图像");
	}
	~Fourier()override {}

	void operator()()override;

};


}//!namespace digital


#endif // !_SOLUTION_FOURIER_H_
