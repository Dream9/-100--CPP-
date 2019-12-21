#ifndef _SOLUTION_CANNY_H_
#define _SOLUTION_CANNY_H_ 

#include"Solution/solution.h"

namespace digital {

//brief:
class Canny : public Solution {
public:
	//paramter:�μ�opencv::canny��������
	Canny(const string& path,
		double bottom,
		double top,
		int size = 3,
		bool use_L2 = false,
		bool flag = false,
		const string& name = "Canny")
		:Solution(name, path, flag),
		threshold_1_(bottom),
		threshold_2_(top),
		aperture_size_(size),
		use_L2gradient_(use_L2)
	{
		getDescriptionHandler().assign("ʹ��Canny���б�Ե���");
	}
	~Canny()override {}

	void operator()();

private:
	double threshold_1_;
	double threshold_2_;
	int aperture_size_;
	bool use_L2gradient_;
};
}//!namespace digital
#endif // !_SOLUTION_CANNY_H_
