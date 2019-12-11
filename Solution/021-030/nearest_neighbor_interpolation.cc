#include"Solution/021-030/nearest_neighbor_interpolation.h"
#include"Solution/geometric_transform.h"

#include<opencv2/highgui.hpp>

namespace {

const double kScale = 1.5;

}
namespace digital {

//brief:
//becare:����ʵ��λ��geometric_transform.[h/cc]��
void NearestNeighborInterpolation::operator()() {
	cv::Mat data = cv::imread(getPath());
	if (data.empty()) {
		dealException(kFileError);
		return;
	}

	cv::Mat img;

	//��÷���任����
	cv::Mat M = detail::getRotationMatrix2D(cv::Point(0, 0), 0, kScale, kScale);

	cv::Mat dd = cv::getRotationMatrix2D(cv::Point(0, 0), 0, kScale);
	__MatrixTest(&dd);
	__MatrixTest(&M);

	cv::Size size = data.size();
	size.width = static_cast<int>(kScale * size.width);
	size.height = static_cast<int>(kScale * size.height);

	//���з���任
	detail::warpAffine(data, img, M, size, cv::INTER_NEAREST, cv::Scalar::all(128));


	//cv::Mat test;
	//cv::warpAffine(data, test, dd, size, cv::INTER_NEAREST);
	//show(&data, &test);

	if (needShowOriginal())
		show(&data, &img);
	else
		show(&img);

}


}