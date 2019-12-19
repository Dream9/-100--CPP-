#ifndef _SOLUTION_CANNY_H_
#define _SOLUTION_CANNY_H_ 

#include"Solution/solution.h"

namespace digital {

//brief:
class Canny : public Solution {
public:
	//paramter:参见opencv::canny参数解释
	Canny(const string& path,
		double bottom,
		double top,
		int size = 3,
		bool L2 = false,
		bool flag = false,
		const string& name = "Canny")
		:Solution(name, path, flag),
		threshold_1_(bottom),
		threshold_2_(top),
		aperture_size_(size),
		L2gradient(L2)
	{
		getDescriptionHandler().assign("使用Canny进行边缘检测");
	}
	~Canny()override {}

	void operator()();

private:
	double threshold_1_;
	double threshold_2_;
	int aperture_size_;
	bool L2gradient;
};
}//!namespace digital
#endif // !_SOLUTION_CANNY_H_
