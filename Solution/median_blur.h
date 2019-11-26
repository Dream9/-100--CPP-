#ifndef _SOLUTION_MEDIAN_BLUR_H_
#define _SOLUTION_MEDIAN_BLUR_H_

#include"Solution/solution.h"

namespace digital {

//brief:��ֵ�˲�
//     ��Ϊ����������˲���������ֵ�˲��ʺϴ�����������
class MedianBlur :public Solution {
public:
	//parameter: win:���ڴ�С���ڴ�����
	MedianBlur(const string& path, int win=5, bool flag =false, const string& name="MedianBlur")
		:Solution(name,path,flag),win_(win)
	{
		assert(win_ > 0);
		assert((win_ & 0x1) == 0x1);

		getDescriptionHandler().assign("ʹ����ֵ�˲�����ͼ��");
	}

	~MedianBlur()override {}

	void operator()()override;

private:
	//������ֵ�˲�
	void fastMedianBlur_(void* src, void* dst, int win);
	int getMedianFromPdf_(int* arr, int half);

	static const int kThresholdForFastMedianBlur;

	int win_;
};


}//!namespace digital


#endif // !_SOLUTION_MEDIAN_BLUR_H_
