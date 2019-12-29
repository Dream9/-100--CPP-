//�ṩ�йؼ�����״ƥ�����صķ�װ

#include<opencv2/core.hpp>

namespace detail{

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
//becare:�����жϴ���һ����������ԭ���ͱ�׼�任һ�£�ֻ�������ڱ��������оͻ���ǰ��ȡ���޳��߶�
void HoughLinesP(cv::InputArray src,
	cv::OutputArray lines,
	double rho,
	double theta,
	int threshold,
	double min_line_length = 0,
	double max_line_length = 0);

}//!namespace detail