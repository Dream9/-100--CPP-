//�ṩ�йؼ�����״ƥ�����صķ�װ

#include<opencv2/core.hpp>

namespace detail{

//brief:ƥ�䷽������opecv����
enum TemplateMatchModes {
    TM_SQDIFF        = 0, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')-I(x+x',y+y'))^2\f]
    TM_SQDIFF_NORMED = 1, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y')-I(x+x',y+y'))^2}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
    TM_CCORR         = 2, //!< \f[R(x,y)= \sum _{x',y'} (T(x',y')  \cdot I(x+x',y+y'))\f]
    TM_CCORR_NORMED  = 3, //!< \f[R(x,y)= \frac{\sum_{x',y'} (T(x',y') \cdot I(x+x',y+y'))}{\sqrt{\sum_{x',y'}T(x',y')^2 \cdot \sum_{x',y'} I(x+x',y+y')^2}}\f]
    TM_CCOEFF        = 4, //!< \f[R(x,y)= \sum _{x',y'} (T'(x',y')  \cdot I'(x+x',y+y'))\f]
                          //!< where
                          //!< \f[\begin{array}{l} T'(x',y')=T(x',y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} T(x'',y'') \\ I'(x+x',y+y')=I(x+x',y+y') - 1/(w  \cdot h)  \cdot \sum _{x'',y''} I(x+x'',y+y'') \end{array}\f]
    TM_CCOEFF_NORMED = 5, //!< \f[R(x,y)= \frac{ \sum_{x',y'} (T'(x',y') \cdot I'(x+x',y+y')) }{ \sqrt{\sum_{x',y'}T'(x',y')^2 \cdot \sum_{x',y'} I'(x+x',y+y')^2} }\f]

	TM_ABSDIFF       = 6, //ע�⣬����Ƕ�opencv����չ��cv::matchTemplate����֧�ֱ�����
	TM_ABSDIFF_NORMED= 7, //ע�⣬����Ƕ�opencv����չ��cv::matchTemplate����֧�ֱ�����
};

//brief:�����׼ֱ�߼��
//parameter:src:����ͼ��
//          lines:��������ע��������Ƕ�Ӧֱ�ߵ�(rho,theta)ֵ��������std::vector<cv::vector2f>��Ϊ���������
//          rho:����ֱ�߿ռ��о����step
//          theta:����ֱ�߿ռ��нǶȵ�step,��λ����
//          int threshold:�ж�Ϊֱ�ߵ���ֵ
void HoughLines(cv::InputArray src,
	cv::OutputArray lines,
	double rho,
	double theta, 
	int threshold);


//brief:���ʻ���ֱ�߼��
//parameter:min_line_length / max_line_length:��С/����߶εĳ������ƣ�Ĭ��û������
//          ���������μ�HoughLines��������
//becare:�����жϴ���һ����������ԭ��ͱ�׼�任һ�£�ֻ�������ڱ��������оͻ���ǰ��ȡ���޳��߶�
void HoughLinesP(cv::InputArray src,
	cv::OutputArray lines,
	double rho,
	double theta,
	int threshold,
	double min_line_length = -1,
	double max_line_length = -1);

//brief:���ݱ�׼����任����Լ�ԭʼͼ���Լ��߽����룬�������߶�
//      ��Ҫ����Ϊ��׼�任�õ��Ľ���Ƕ�ֱ����������������
//parameter: original:ԭʼͼ��
//           mask:�߽����ݣ�ͨ����Canny�ȱ�Ե���Ķ�ֵ�����
//           hough��houghLines���
//           dst�����
//           val:�߽���ɫ
void overlapHoughImage(cv::InputArray original,
	cv::InputArray mask,
	cv::InputArray hough, 
	cv::OutputArray dst,
	cv::Scalar val = cv::Scalar(0,0,255));

//brief��ģ��ƥ��
//parameter��src:��ƥ������ͼ��ֻ����CV_8UC1����չ
//           templ��ģ��ͼ��
//           dst:������, type()ΪCV_32FC1
//           method:ƥ��ģʽ������
void matchTemplate(cv::InputArray src,
	cv::InputArray templ,
	cv::OutputArray dst,
	int method);

}//!namespace detail