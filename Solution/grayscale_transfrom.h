//brief:�ṩ�йػҶȱ任�Ĺ��ܷ�װ����Ҫ�����Ҷȷ�ת��ֱ��ͼͳ��ƥ��Ƚ��漰�����Ҷ�ֵ��ͼ������

#ifndef _SOLUTION_GRAYSCALE_TRANSFORM_H_
#define _SOLUTION_GRAYSCALE_TRANSFORM_H_

#include<opencv2/core.hpp>
#include<functional>

namespace detail {

typedef std::function<void(uint8_t* cursor)> GrayScaleOperationType;

//brief:���ұ�
//     ������ڴ����������ظ����㣬����lut���Խ�ʡ��Щ�������
//paramter:lut:�μ�cv::LUT,�ر�ʱ����ͨ����ƥ�䷽��
void LUT(cv::Mat& src, 
	cv::Mat& dst, 
	cv::Mat& lut);

//brief:
double otsuThreshold(cv::Mat& src);

//brief:���㵥��Mat�ĸ����ܶȺ���
//parameter:src:������Դ
//          dst:�洢Ŀ�꣬���Ĭ��ΪCV_32F
//          min,max:����Դ�ķ�Χ
//          bins:�ֿ���
//          *select_channels ��len����˵����������Mat��ͨ����Ĭ��ʱΪȫ���������㣬����֯��ʽ�μ�cv::calcHist
//becare:Mat��depth������CV_8U,opencv��������CV_16U��CV_32F�����أ�ͨ��if-else�ַ���
void calcHistogram(const cv::Mat& src,
	cv::Mat& dst,
	int min,
	int max,
	int bins,
	int* select_channels = nullptr,
	int len = -1);

//brief:���ݻҶ�pdf����Ŀ��ͼ��
//paramter: hist:r*1��pdf����
//          dst:������ݴ洢λ��
//          size��type Ŀ������
//          color:��ɫ
void fillHistogram(const cv::Mat& hist,
	cv::Mat& dst,
	const cv::Size& size,
	int type = CV_8UC1,
	const cv::Scalar& color = cv::Scalar::all(0));

//brief:�Ҷȷ�ת
//paramter:src:Ŀ�����
//         max_value:ͼ�����ֵ
void colorInversion(cv::Mat& src, int max_value = UINT8_MAX);

//brief:Ϊÿ������ֵӦ���û�����Ĳ���ops
//paramter:dst:Ŀ�����
//         ops:�û�����Ŀɵ��ö������Ͳμ�����
//          not_merge_channel:��Ҫ��Բ��ֱ�����Ҫ��һ��λ�ô��Ķ��Ԫ��ֵһͬ����(���統������)����ʱ���ܲ�����ǵ�������
//                            Ĭ�ϲ�ִ����ڼ�����λ���õ�������
//becare:���ǽ��漰���������صĲ����������϶�����ͨ�����ӿ���ɣ���Ȼgamma�任���Ҷ����졢���⻯��
void grayscaleTransform(cv::Mat& src, const GrayScaleOperationType& ops, bool not_merge_channel = false);

//brief:�μ�cv::convertScaleAbsʵ��
//     ������������lambda�������grayscalTransformʵ��
//becare:����grayscaleTransformֻ�ػ���CV_8U,��˱�����Ҳֻ���srcΪCV_8U���͵�����
void convertScaleAbs(cv::Mat& src, cv::Mat& dst, double alpha, double beta);

//brief:�μ�cv::equalizeHistʵ��
//becare:opencv��Ҫ��src������CV_8U1C�����ݣ���������һ��С��չ������ͨ������Ҳ���Բ�������
void equalizeHist(cv::Mat& src, cv::Mat& dst);

//brief:
//void addAbs(cv::InputArray first, cv::InputArray second, cv::OutputArray dst);

}//!namespace detail



#endif // !_SOLUTION_GRAYSCALE_TRANSFORM_H_
