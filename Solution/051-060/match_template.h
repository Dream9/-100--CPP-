#ifndef _SOLUTION_MATCH_TEMPLATE
#define _SOLUTION_MATCH_TEMPLATE

#include"Solution/solution.h"

namespace digital {

//brief:分别采用了6种传统方法进行匹配
class MatchTemplate : public Solution {
public:
	
	//匹配方法
	enum TemplateMatchModes {
		TM_SQDIFF = 0, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')-I(x+x',y+y'))^2\f]
		TM_SQDIFF_NORMED = 1, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y')-I(x+x',y+y'))^2}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
		TM_CCORR = 2, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')  \cdot I(x+x',y+y'))\f]
		TM_CCORR_NORMED = 3, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y') \cdot I(x+x',y+y'))}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
		TM_CCOEFF = 4, //!< \f[R(x,y)= \sum _{x',y'} (T'(x',y')  \cdot I'(x+x',y+y'))\f]
							  //!< where
							  //!< \f[\begin{array}{l} T'(x',y')=T(x',y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} T(x'',y'') \\ I'(x+x',y+y')=I(x+x',y+y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} I(x+x'',y+y'') \end{array}\f]
		TM_CCOEFF_NORMED = 5, //!< \f[R(x,y)= \frac{ \sum_{x',y'} (T'(x',y') \cdot I'(x+x',y+y')) }{ \sqrt{\sum_{x',y'}T'(x',y')^2 \cdot \sum_{x',y'} I'(x+x',y+y')^2} }\f]

	    TM_ABSDIFF       = 6,  //注意，这个是对opencv的扩展，cv::matchTemplate并不支持本参数
	    TM_ABSDIFF_NORMED= 7,  //注意，这个是对opencv的扩展，cv::matchTemplate并不支持本参数
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
		getDescriptionHandler().assign("使用传统的模板匹配方法识别图像,注意这些方法不具有尺寸和旋转的不变性");
	}
	~MatchTemplate()override {}

	void operator()()override;

private:
	string template_path_;
	int op_;
};

}//!namespace digital
#endif // !_SOLUTION_MATCH_TEMPLATE
