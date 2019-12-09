#ifndef _SOLUTION_EQUALIZE_HISTOGRAM_H_
#define _SOLUTION_EQUALIZE_HISTOGRAM_H_

#include"Solution/solution.h"

namespace digital {

//brief:ʵ��ֱ��ͼ���⻯
//     ���ڸ�����ͼ�񣬾��⻯�Ľ����Ψһ�ģ���Ҳ��ֱ��ͼƥ������ۻ���
class EqualizeHistogram : public Solution {
public:
	EqualizeHistogram(const string& path, bool show_pdf = false, bool flag = false, const string& name = "EqualizeHistogram")
		:Solution(name, path, flag), show_pdf_(show_pdf)
	{
		getDescriptionHandler().assign("ͨ��ֱ��ͼ���⻯����ͼ��̬��Χ���ḻ��Ҷ�ϸ��");
	}
	~EqualizeHistogram()override {}

	void operator()() override;

private:
	bool show_pdf_;

};

}//!namespace digital

#endif // !_SOLUTION_EQUALIZE_HISTOGRAM_H_
