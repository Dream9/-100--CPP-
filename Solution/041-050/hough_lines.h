#ifndef _SOLUTION_HOUGH_LINES_H
#define _SOLUTION_HOUGH_LINES_H

#include"Solution/solution.h"

namespace digital {

//brief:
class HoughLines : public Solution {
public:
	//parameter:rho,theta,threshold�μ�HoughLines���������������μ�Solution
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
		getDescriptionHandler().assign("ͨ������ֱ�߱任̽��ͼ���е�ֱ��");
	}
	~HoughLines()override {}

	void operator()()override;

private:
	//�йػ���ֱ��̽��Ĳ���
	double rho_;
	double theta_;
	int threshold_;

};

}//!namespace digital

#endif // !_SOLUTION_HOUGH_LINES_H
