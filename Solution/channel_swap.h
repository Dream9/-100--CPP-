#ifndef _SOLUTION_CHANNEL_SWAP_H_
#define _SOLUTION_CHANNEL_SWAP_H_ 

#include"Solution/solution.h"

#include<opencv2/highgui.hpp>

#include<iostream>

namespace digital {

//brief:
class ChannelSwap :public Solution {
public:
	ChannelSwap(const string& path,
				bool show_original = false,
				const string& name = "ChannelSwap")
		:Solution(name, path, show_original)
	{
		Solution::getDescriptionHandler().assign("��ȡͼ��Ȼ�� RGB ͨ���滻�� BGR ͨ����");
	}
	virtual ~ChannelSwap(){}


	virtual void operator()() {
		cv::Mat data = cv::imread(getPath(), 1);
		//assert(!data.empty());

		if (data.empty())
			return dealException(kFileError);

		cv::Mat img = needShowOriginal() ? data.clone() : data;

		int rows = img.rows;
		int cols = img.cols;
		int channels = img.channels();

		//brief:��Ҫ����Ϥһ��Mat���ڴ�ṹ
		//      ����Ĳ������ǱȽ����ģ�ֻҪ�����ǰָ�뼰���ݽṹ
		for (int i = 0; i < rows; ++i) {
			//for (int j = 0; j < cols; ++j) {
			//  //ͨ��ָ��cv::Vec3b��ָ�����ĳ��������
			//	//auto ptr = img.ptr<cv::Vec3b>(i, j);
			//	//std::swap((*ptr)[0], (*ptr)[2]);
			//
			//  //ͨ��uint8_t*����ĳ�����ص�����
			//	auto ptr = img.ptr<uint8_t>(i,j);
			//	std::iter_swap(ptr, ptr + 2);

			//}

			//������һ����ͨ������һ�з���
			auto ptr = img.ptr<uint8_t>(i);
			for (int j = 0; j < cols; ++j) {
				auto tmp = ptr + j * channels;
				std::iter_swap(tmp, tmp + 2);
			}

			//����Ҳ����ͨ��cv::Mat::at����
			//���߽���data�õ��������ָ�룬Ȼ���ֶ�����ƫ��(step[0],step[1])����
			//�������������
			//���ʶ���һ���ģ��ؼ����˽�Mat�����֯���ݵ�
		}

		//for test
		//__MatrixTest(&data, &img);
		
		if (needShowOriginal())
			show(&data, &img);
		else
			show(&img);
	}

};

}//!namespace digital



#endif