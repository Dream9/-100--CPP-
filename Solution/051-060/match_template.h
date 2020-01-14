#ifndef _SOLUTION_MATCH_TEMPLATE
#define _SOLUTION_MATCH_TEMPLATE

#include"Solution/solution.h"

namespace digital {

//brief:�ֱ������6�ִ�ͳ��������ƥ��
class MatchTemplate : public Solution {
public:
	
	//ƥ�䷽��
	enum TemplateMatchModes {
		TM_SQDIFF = 0, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')-I(x+x',y+y'))^2\f]
		TM_SQDIFF_NORMED = 1, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y')-I(x+x',y+y'))^2}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
		TM_CCORR = 2, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')  \cdot I(x+x',y+y'))\f]
		TM_CCORR_NORMED = 3, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y') \cdot I(x+x',y+y'))}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
		TM_CCOEFF = 4, //!< \f[R(x,y)= \sum _{x',y'} (T'(x',y')  \cdot I'(x+x',y+y'))\f]
							  //!< where
							  //!< \f[\begin{array}{l} T'(x',y')=T(x',y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} T(x'',y'') \\ I'(x+x',y+y')=I(x+x',y+y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} I(x+x'',y+y'') \end{array}\f]
		TM_CCOEFF_NORMED = 5, //!< \f[R(x,y)= \frac{ \sum_{x',y'} (T'(x',y') \cdot I'(x+x',y+y')) }{ \sqrt{\sum_{x',y'}T'(x',y')^2 \cdot \sum_{x',y'} I'(x+x',y+y')^2} }\f]

	    TM_ABSDIFF       = 6,  //ע�⣬����Ƕ�opencv����չ��cv::matchTemplate����֧�ֱ�����
	    TM_ABSDIFF_NORMED= 7,  //ע�⣬����Ƕ�opencv����չ��cv::matchTemplate����֧�ֱ�����
	};

	MatchTemplate(const string& path,
		const string& template_path,
		int op,
		bool flag = false,
		const string& name = "MatchTemplate")
		:Solution(name, path, flag),
		template_path_(template_path),
		op_(op)
	{
		getDescriptionHandler().assign("ʹ�ô�ͳ��ģ��ƥ�䷽��ʶ��ͼ��,ע����Щ���������гߴ����ת�Ĳ�����");
	}
	~MatchTemplate()override {}

	void operator()()override;

private:
	string template_path_;
	int op_;
};

}//!namespace digital
#endif // !_SOLUTION_MATCH_TEMPLATE
