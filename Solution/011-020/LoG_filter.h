#ifndef _SOLUTION_LOG_H_
#define _SOLUTION_LOG_H_

#include"Solution/solution.h"

namespace digital {

//brief:Laplacian of Gausian�˲�
class LoG : public Solution {
public:
	LoG(const string& path, int win = 3, bool flag = false, const string& name = "LoG")
		:Solution(name, path, flag), win_(win <= 1 ? 3 : win)
	{
		assert((win_ & 0x1) == 0x1);
		getDescriptionHandler().assign("ʹ��LoG�˲�");
	}

	~LoG()override {}

	void operator()()override;

private:
	void getLoGFilter_(void*);

	const int win_;
};
}

#endif // !_SOLUTION_LOG_H_
