#ifndef _SOLUTION_GAMMA_CORRECTION_H_
#define _SOLUTION_GAMMA_CORRECTION_H_

#include"Solution/solution.h"

namespace digital {

//brief:韦伯-费希纳定律是表明心理量和物理量之间关系的定律，即感觉的差别阈限随原来刺激量的变化而变化，
//     而且表现为一定的规律性，用公式来表示，就是△Φ/Φ=C，其中Φ为原刺激量，△Φ为此时的差别阈限，
//     C为常数，又称为韦柏率
//     伽马变换的目的就是韦伯定理的例子
class GammaCorrection : public Solution {
public:
	GammaCorrection(const string& path, 
		double c = 1.5,
		double gamma = 2.5 ,
		bool flag = false,
		const string& name = "GammaCorrection")
		:Solution(name, path, flag),
		c_(c),
		gamma_(gamma)
	{
		getDescriptionHandler().assign("配合电子设备的幂律响应");
	}
	~GammaCorrection()override {}

	void operator()()override;

private:
	double  c_;
	double gamma_;
};

}//!namespace digital


#endif // !_SOLUTION_GAMMA_CORRECTION_H_
