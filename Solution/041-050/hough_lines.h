#ifndef _SOLUTION_HOUGH_LINES_H
#define _SOLUTION_HOUGH_LINES_H

#include"Solution/solution.h"

namespace digital {

//brief:
class HoughLines : public Solution {
public:
	//parameter:rho,theta,threshold参见HoughLines参数，其他参数参见Solution
	HoughLines(const string& path,
		double rho,
		double theta,
		int threshold, 
		bool flag = false,
		const string& name = "HoughLines")
		:Solution(name, path, flag), rho_(rho), theta_(theta), threshold_(threshold)
	{
		assert(rho_ > 0);
		assert(theta_ > 0);
		assert(threshold_ > 0);
		getDescriptionHandler().assign("通过霍夫直线变换探测图像中的直线");
	}
	~HoughLines()override {}

	void operator()()override;

private:
	//有关霍夫直线探测的参数
	double rho_;
	double theta_;
	int threshold_;

};

}//!namespace digital

#endif // !_SOLUTION_HOUGH_LINES_H
