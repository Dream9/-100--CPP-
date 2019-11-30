#ifndef _SOLUTION_H_
#define _SOLUTION_H_

#include"Solution/type_extension.h"

namespace digital {

//brief:问题解决的通用接口
class Solution {
public:
	Solution(const string& name, const string& path, bool flag)
		: question_name_(name),
		path_(path), 
		show_old_(flag),
		description_()
	{
		assert(!name.empty());
		assert(!path.empty());
	}

	virtual ~Solution() {
		;
	}
	virtual void operator()() = 0;

	string getDescription()const {
		return description_;
	}
	string& getDescriptionHandler() {
		return description_;
	}

	const string& getPath()const {
		return path_;
	}
	const string& getName()const {
		return question_name_;
	}
	
	void show(void* fig);
	void show(void* fig, const string& str);
	void show(void* fig, void* fig_2);
	void show(void* fig, void* fig_2, const string& str);
	void show(void** figs, int len);
	void show(void** figs, int len, const string& str);

	bool needShowOriginal()const {
		return show_old_;
	}
	

private:

	string question_name_;
	string path_;
	bool show_old_;
	
	string description_;
};

//统一的接口
void Solve(Solution& solution);
//brief:仅用于测试
void __MatrixTest(void* Mat1, void* Mat2);

}


#endif
