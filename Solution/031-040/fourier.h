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
		getDescriptionHandler().assign("����fourier�任���õ���Ƶ��ͼ����λͼ��Ȼ��"
			"���з��任�õ�ԭʼͼ��");
	}
	~Fourier()override {}

	void operator()()override;

};


}//!namespace digital


#endif // !_SOLUTION_FOURIER_H_
