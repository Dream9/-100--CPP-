#include"Solution/061-070/hog.h"
#include"Solution/descriptor.h"

#include<opencv2/highgui.hpp>
#include<opencv2/objdetect.hpp>

namespace digital {

void Hog::operator()() {
	cv::Mat data = cv::imread(getPath(), cv::IMREAD_GRAYSCALE);
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

#ifndef USE_OPENCVLIN
	
	detail::HOGDescriptor hog(cv::Size(64, 64), cv::Size(16, 16), \
		cv::Size(8, 8), cv::Size(8, 8), 3);//���ù��캯����������ֵ

#else

	//cvʵ��
	cv::HOGDescriptor hog(cv::Size(64, 64), cv::Size(16, 16), \
		cv::Size(8, 8), cv::Size(8, 8), 3);//���ù��캯����������ֵ
	
#endif
	//ֻ���ͼ��Ψһ��������ʱ����Ҫresizeһ��
	//resize(src, src, Size(64, 64));
	std::vector<float> descriptors;//HOG����������
	hog.compute(data, descriptors);

	//��������ֵ
	size_t dd = hog.getDescriptorSize();
	//double sdf = hog.getWinSigma();
	size_t ss = descriptors.size();
	
	coutInfo("test");
}

}//!namespace digital