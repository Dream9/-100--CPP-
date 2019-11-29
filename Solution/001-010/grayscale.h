#ifndef _SOLUTION_GRAYSCALE_H_
#define _SOLUTION_GRAYSCALE_H_ 

#include"Solution/solution.h"


namespace digital {

//brief:
class Grayscale :public Solution {
public:
	Grayscale(const string& path, bool show_old, const string& name = "Grayscale")
		:Solution(name, path, show_old) 
	{
		getDescriptionHandler().assign("²ÊÉ«Í¼Ïñ»Ò¶È»¯");
	}

	~Grayscale() override {}

	void operator()() override;
	
};
}





#endif