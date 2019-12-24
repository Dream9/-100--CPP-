#include"Solution/base.h"
#include"Solution/type_extension.h"
#include"Solution/geometric_transform.h"

#include<opencv2/imgproc.hpp>

#include<random>
#include<vector>

namespace {

typedef std::function<double(double, double)> NormType;

const double kHalf45 = 22.5;

//brief:non-maximum suppression
void __NMS(cv::Mat& dx, cv::Mat& dy, cv::Mat& dst, cv::Mat& gradient);
cv::Mat __hysteresisThreshold(cv::Mat& src, double lower, double upper);
void __dfs(cv::Mat& src, cv::Mat& dst, double lower, int row, int column);
}

namespace detail {

static std::default_random_engine __global_eng;
static std::normal_distribution<> __global_dis(1, 1);
//brief:���ط�����̬�ֲ����������E = 1 , D = 1
//becare:�����̰߳�ȫ��
//      Ҳ���Բ���RNG��������
double randNorm() {
	return __global_dis(__global_eng);
}

//breif:����׳�
//FIXME:β�ݹ�ʵ��
int factorial(int i) {
	assert(i > -1);

	if (i == 0 || i == 1)
		return 1;
	return i * factorial(i - 1);
}


//brief:�Զ���ʽչ��ʽ��Ϊ�ǹ�һ����˹ϵ���Ľ���
//return:������ӽ��Ϊ������
cv::Mat getSmoothKernel(int n)
{
	cv::Mat kernel = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	auto cur = kernel.ptr<float>(0, 0);
	for (int i = 0; i < n; i++){
		//���ö���ʽչ��ʽ��Ϊ��˹ƽ���Ľ���
		*cur++ = static_cast<float>(detail::factorial(n - 1) / (detail::factorial(i) * detail::factorial(n - 1 - i)));
	}
	return kernel;
}

//brief:�õ�soble�������,
//return:������ӽ��Ϊ������
cv::Mat getSobelDifference(int n)
{
	cv::Mat diff = cv::Mat::zeros(cv::Size(n, 1), CV_32FC1);
	//������ӵ�ǰ����n-2�׵Ķ���ʽչ��ʽ����ƽ���������1
	cv::Mat prev = getSmoothKernel(n - 1);
	//���ò�����ӵĻ�ù��̣��ı�Mat
	auto cur = diff.ptr<float>(0, 0);
	auto iter = prev.ptr<float>(0, 0);

	//FIXME:opencv����Sobel������ǰ-����-�£��ļ��㷽ʽ
	//*cur++ = -1;
	//for (int i = 1; i < n - 1; i++)
	//{
	//	*cur++ = (*iter - *(iter + 1));
	//	++iter;
	//}
	//*cur = 1;
	
	*cur++ = 1;
	for (int i = 1; i < n - 1; i++)
	{
		*cur++ = (*(iter+1) - *iter);
		++iter;
	}
	*cur = -1;
	return diff;
}

//brief:��ά���
void convolution2D(const cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth, 
	const cv::Mat& kernel, 
	cv::Point p , 
	int bordertype) 
{
	assert(!src.empty());
	cv::Mat flip_k;
	cv::flip(kernel, flip_k, -1);

	//fort test
	//std::vector<float> vk = std::vector<float>(flip_k);

	cv::filter2D(src, dst, ddepth, flip_k, p, 0.0, bordertype);
}

//brief:����Ķ�ά���
void sepConvolution2D(const cv::Mat& src, 
	cv::Mat& dst, 
	int ddepth,
	const cv::Mat& kernelx,
	const cv::Mat& kernely, 
	cv::Point p , 
	int bordertype) 
{
	assert(!src.empty());
	cv::Mat flip_kx;
	cv::Mat flip_ky;
	cv::flip(kernelx, flip_kx, -1);
	cv::flip(kernely, flip_ky, -1);

	//fort test
	//std::vector<float> vx = std::vector<float>(kernelx);
	//std::vector<float> vy = std::vector<float>(flip_ky);

	cv::sepFilter2D(src, dst, ddepth, flip_kx, flip_ky, p, 0.0, bordertype);
}

//brief:����׼����ʽ
//becare:�������ͷַ����ɵ����ߴ����op����ַ�
void filter2DNonLinear(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, int ddepth, NonlinearOperationType op,
	cv::Point anchor, int borderType) {
	cv::Size ksize = kernel.size();
	anchor.x = anchor.x == -1 ? (ksize.width >> 1) : anchor.x;
	anchor.y = anchor.y == -1 ? (ksize.height >> 1) : anchor.y;

	//becare:����Ҫ��ê��ʼ��λ���ڲ�
	assert(anchor.x < ksize.width && anchor.x >= 0);
	assert(anchor.y < ksize.height && anchor.y >= 0);
	int offset_t = anchor.y;
	int offset_b = ksize.height - anchor.y - 1;
	int offset_l = anchor.x;
	int offset_r = ksize.width - anchor.x - 1;

	cv::Size ssize = src.size();
	dst = cv::Mat(ssize, CV_MAKETYPE(ddepth, src.channels()));

	//��չ�߽�
	cv::Mat data;
	cv::copyMakeBorder(src, data, offset_t, offset_b, offset_l, offset_r, borderType);

	//�˲�����
	auto cur = dst.data;
	std::vector<uint8_t*> arr_dst(ksize.height, nullptr);
	cv::Size size = data.size();
	size_t kStep = data.step;
	auto src_elemSize = src.elemSize();
	auto dst_elemSize = dst.elemSize();

	for (int i = offset_t; i < size.height - offset_b; ++i) {
		auto cursor = data.ptr(i - offset_t, 0);//cursorָ�򴰿����ϴ���λ��
		for (int j = offset_l; j < size.width - offset_r; ++j) {

			assert(cursor == data.ptr(i - offset_t, j - offset_l));
			assert(cur == dst.ptr(i - offset_t, j - offset_l));

			//�������ڼ���
			auto iter = cursor;
			int n = 0;
			for (int x = 0; x < ksize.height; ++x) {
				assert(iter == data.ptr(i - offset_t + x, j - offset_l));

				arr_dst[x] = iter;
				iter += kStep;
				
			}

			//�����û�����
			op(&arr_dst[0], cur);

			cur += dst_elemSize;
			cursor += src_elemSize;
		}
	}

}

//brief:Sobel���ӣ����÷���ʽ��������
void Sobel(cv::Mat& src, cv::Mat& dst, int ddepth, int dx, int dy, int win) {
	assert((win & 0x1) == 0x1);
	assert(!src.empty());

	cv::Mat smooth_kernel = detail::getSmoothKernel(win);
	//���ú����õ��������
	cv::Mat diff_kernel=detail::getSobelDifference(win);

#ifndef NDEBUG
	//for test
	std::vector<int> v = std::vector<int>(smooth_kernel);
	std::vector<int> v2 = std::vector<int>(diff_kernel);
#endif

	if (dx)
		detail::sepConvolution2D(src, dst, ddepth, diff_kernel, smooth_kernel.t());

	if(dx==0 && dy)
		detail::sepConvolution2D(src, dst, ddepth, smooth_kernel.t(), diff_kernel);

}

//brief:Canny�߽���
//      ������sobel����֮�ϣ������˷Ǽ���ֵ�����Լ�˫��ֵ���ͺ���ֵ����
void Canny(cv::Mat& src, cv::Mat& dst, double threshold_1, double threshold_2,
	int apertureSize , bool useL2gradient) {

	assert(src.type() == CV_8UC1);
	assert((apertureSize & 0x1) == 0x1);
	assert(threshold_1 < threshold_2);
	assert(threshold_1 >= 0);
	assert(!src.empty());

	//FIXME:�˲�������
	cv::Mat data;
	cv::GaussianBlur(src, data, cv::Size(3, 3), -1);

	cv::Mat dx;
	cv::Mat dy;

#ifdef USE_OPENCVLIB
	cv::Sobel(data, dx, CV_64F, 1, 0, apertureSize);
	cv::Sobel(data, dy, CV_64F, 0, 1, apertureSize);
#else
	detail::Sobel(data, dx, CV_64F, 1, 0, apertureSize);
	detail::Sobel(data, dy, CV_64F, 0, 1, apertureSize);
#endif

	cv::Mat gradient;
	if (useL2gradient) 
		cv::magnitude(dx, dy, gradient);
	else 
		cv::addWeighted(dx, 0.5, dy, 0.5, 0, gradient, CV_64F);

#ifdef SHOW_PROCESS
	cv::Mat tmp;
	cv::convertScaleAbs(gradient, tmp);
	cv::imshow("gradient", tmp);
	cv::waitKey(0);
	cv::destroyAllWindows();
#endif

	//�Ǽ���ֵ����
	cv::Mat edge;
	__NMS(dx, dy, edge, gradient);

#ifdef SHOW_PROCESSS
	cv::convertScaleAbs(edge, tmp);
	cv::namedWindow("nms", cv::WINDOW_NORMAL);
	cv::imshow("nms", tmp);
	cv::waitKey(0);
	cv::destroyAllWindows();

	double min, max;
	cv::minMaxLoc(edge, &min, &max);
	auto scalar = cv::mean(edge);
	digital::__printInfo("min:%f,max:%f, mean:%f", min, max, scalar[0]);

#endif
	//˫��ֵ���ͺ���ֵ����
	dst = __hysteresisThreshold(edge, threshold_1, threshold_2);
}



}//!namespace detail


namespace {


//brief:�Ǽ���ֵ����
//TODO:�������Բ�ֵ�ķ�ʽ����б𾫶�
void __NMS(cv::Mat& dx, cv::Mat& dy, cv::Mat& dst, cv::Mat& gradient) {
	assert(dx.type() == CV_64FC1);
	assert(dy.type() == CV_64FC1);
	assert(gradient.type() == CV_64FC1);
	assert(dx.isContinuous());
	assert(dy.isContinuous());
	assert(gradient.isContinuous());

	cv::Size size = dx.size();
	dst = cv::Mat::zeros(size, dx.type());
	
	double* ptry = dy.ptr<double>(1, 1);
	double* ptrx = dx.ptr<double>(1, 1);
	double* ptrg = gradient.ptr<double>(1, 1);
	double* iter = dst.ptr<double>(1, 1);

	//becare:ע��Ƕȵ�����ϵ
	for (int y = 1; y < size.height - 1; ++y) {
		for (int x = 1; x < size.width - 1; ++x) {
			double angle = detail::Rad2Deg(std::atan2(*ptry, *ptrx));

			//FIXME���ò�ֵ�ķ�ʽ�����Ե����ж���ȷ��
			//δ�������Բ�ֵ��ʵ�ַ�ʽ
			if (std::fabs(angle) <= kHalf45 || std::fabs(angle) >= 180 - kHalf45)
				//���ҽ���
				*iter = (ptrg[0] > ptrg[1] && ptrg[0] > ptrg[-1]) ? ptrg[0] : 0.0;
			else if ((angle >= 90 - kHalf45 && angle <= 90 + kHalf45) ||
				(angle >= -90 - kHalf45 && angle <= -90 + kHalf45))
				//���½���
				*iter = (ptrg[0] > ptrg[size.width] && ptrg[0] > ptrg[-size.width]) ? ptrg[0] : 0.0;
			else if ((angle >= -45 - kHalf45 && angle <= -45 + kHalf45) ||
				(angle >= 135 - kHalf45 && angle <= 135 + kHalf45))
				//��������
				*iter = (ptrg[0] > ptrg[size.width - 1] && ptrg[0] > ptrg[-size.width + 1]) ? ptrg[0] : 0.0;
			else if ((angle >= 45 - kHalf45 && angle <= 45 + kHalf45) ||
				(angle >= -135 - kHalf45 && angle <= -135 + kHalf45))
				//��������
				*iter = (ptrg[0] > ptrg[size.width + 1] && ptrg[0] > ptrg[-size.width - 1]) ? ptrg[0] : 0.0;
			else
				assert(0);//it should not be here

				
			++iter;
			++ptrg;
			++ptrx;
			++ptry;
		}
	}

}

//brief:����˫��ֵ���ͺ���ֵ����
//      ���ʾ���dfs����ͼ
cv::Mat __hysteresisThreshold(cv::Mat& src, double lower, double upper) {
	assert(src.type() == CV_64FC1);
	assert(src.isContinuous());

	//���ǽ����Ҳ����Ϊvisited�ı��
	cv::Mat out = cv::Mat::zeros(src.size(),CV_8UC1);
	cv::Size size = src.size();

	double* cursor = src.ptr<double>(0, 0);

	for (int y = 0; y < size.height; ++y) {
		for (int x = 0; x < size.width; ++x) {
			assert(cursor == src.ptr<double>(y, x));
			//digital::__printInfo("index: y :%d,x:%d,val:%f\r\n", y,x,*cursor);
			if (*cursor < lower) {
				++cursor;
				continue;
			}

			if (*cursor > upper) {
				//digital::__printInfo("��ʼ�ݹ�%f\r\n", *cursor);
				__dfs(src, out, lower, y, x);
			}

			++cursor;
		}
	}


	return out;
}

#define CHECK_OUT_RANGE(row,column,height, width)(column < 0 || row < 0 || column >= width || row >= height)

//brief:
void __dfs(cv::Mat& src, cv::Mat& dst, double lower, int row, int column) {
	if (dst.at<uchar>(row,column) != 0)//�Ƿ������Ѿ����ʹ�
		return;

	dst.at<uchar>(row,column) = UINT8_MAX;

	//���������8��ͨ�ķ�ʽ
	//FIXME�� or use 4?
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			if (CHECK_OUT_RANGE(row + i, column + j, src.rows, src.cols))
				continue;
			if (src.at<double>(row + i, column + j) >= lower) {
				__dfs(src, dst, lower, row + i, column + j);
			}
		}
	}
}

#undef CHECK_OUT_RANGE

}//!namespace 