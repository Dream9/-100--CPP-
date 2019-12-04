#ifndef _SOLUTION_EMBOSS_FILTER_H_
#define _SOLUTION_EMBOSS_FILTER_H_

#include"Solution/solution.h"

namespace digital {

//brief:ԭ����ǽ�soble�������ĸ�ֵ���ɸ������Ӱ����ֵ���ɸ���Ĺ�������
class EmbossFilter : public Solution {
public:
	EmbossFilter(const string& path, bool flag = false, const string& name = "EmbossFilter")
		:Solution(name, path, flag)
	{
		getDescriptionHandler().assign("ͨ������Soble��Ե�õ�����Ч��");
	}
	~EmbossFilter()override {}

	void operator()()override;

private:
	static const int kWin;
};
}

#endif // !_SOLUTION_EMBOSS_FILTER_H_
