//�ṩ�й���̬ѧ�任�Ĺ���

#ifndef _SOLUTION_MORPHOLOGY_TRANSFORM_H_
#define _SOLUTION_MORPHOLOGY_TRANSFORM_H_

#include<opencv2/core.hpp>

namespace detail {

//��opencv����
enum MorphTypes {
	MORPH_ERODE = 0, //!< see #erode
	MORPH_DILATE = 1, //!< see #dilate
	MORPH_OPEN = 2, //!< an opening operation
	//!< \f[\texttt{dst} = \mathrm{open} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \mathrm{erode} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_CLOSE = 3, //!< a closing operation
	//!< \f[\texttt{dst} = \mathrm{close} ( \texttt{src} , \texttt{element} )= \mathrm{erode} ( \mathrm{dilate} ( \texttt{src} , \texttt{element} ))\f]
	MORPH_GRADIENT = 4, //!< a morphological gradient
	//!< \f[\texttt{dst} = \mathrm{morph\_grad} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \texttt{src} , \texttt{element} )- \mathrm{erode} ( \texttt{src} , \texttt{element} )\f]
	MORPH_TOPHAT = 5, //!< "top hat"
	//!< \f[\texttt{dst} = \mathrm{tophat} ( \texttt{src} , \texttt{element} )= \texttt{src} - \mathrm{open} ( \texttt{src} , \texttt{element} )\f]
	MORPH_BLACKHAT = 6, //!< "black hat"
	//!< \f[\texttt{dst} = \mathrm{blackhat} ( \texttt{src} , \texttt{element} )= \mathrm{close} ( \texttt{src} , \texttt{element} )- \texttt{src}\f]
	MORPH_HITMISS = 7  //!< "hit or miss"
	//!<   .- Only supported for CV_8UC1 binary images. A tutorial can be found in the documentation
};

enum LineTypes {
	LINE_4 = cv::LINE_4,
	LINE_8 = cv::LINE_8,
};

//brief:��̬ѧ���ͣ�����μ�cv::dilate
//parameter��src:����
//           dst���
//           kernel:�ṹԪSE
//           anchor:SE��ê��
//           iteration����������
void dilate(cv::Mat& src, 
	cv::Mat& dst,
	cv::Mat& kernel,
	cv::Point anchor = cv::Point(-1, -1),
	int iterations = 1);

//breif:��̬ѧ��ʴ
//parameter��src:����
//           dst���
//           kernel:�ṹԪSE
//           anchor:SE��ê��
//           iteration����������
void erode(cv::Mat& src,
	cv::Mat& dst, 
	cv::Mat& kernel, 
	cv::Point anchor = cv::Point(-1, -1),
	int iterations = 1);


//brief:�߼���̬ѧ�任
//parameter:src:����
//          op:ִ�еĲ������Ϸ�ֵ�μ�MorphTypes
//          kernel:�ṹԪSE
//          dst�����
//          anchor:SEê��
//          iteration����������
void morphologyEx(cv::Mat& src,
	cv::Mat& dst, 
	int op, 
	cv::Mat& kernel, 
	cv::Point anchor = cv::Point(-1, -1),
	int iteration = 1);

//brief:���㲢�鲢��ͨ����
//parameter: src:����ͼ��Ҫ��Ϊ��ֵͼ��CV_8UC1
//           dst:���ͼ��
//           flag:��ͨ����Ĭ������ͨ
//           use_dfs:ʹ��dfs������ͨ���жϣ�Ĭ�Ϸ���Ϊdfs��ͼ�����������ع���ʱ����ջ�������
//                   Ŀǰ���Ѿ��Եݹ�������������(kSTACK_THRESHOLD)
//return:�������Ŀ��ܱ�ʶ��id����[1,id]������Ϊ���ܳ��ֵĲ�ͬ�����ı�ʶ
//becare:���ͼ�������嶼�Ƚ�Сʱ(����������kSTACK_THRESHOLD)������use_dfs�ļ����ٶȻ��һ�㣬
//       ����Ĭ�ϲ���quick-union�㷨����Ҫ���߱���
int getConnectComponent(cv::InputArray src,
	cv::OutputArray dst,
	int flag = LineTypes::LINE_4,
	bool use_dfs = false);

//brief:����ÿ�������ͨ��Ŀ
//parameter:src:����ͼ��
//          dst:���ͼ��
//          flag:��ͨ���ͣ�Ĭ�ϲ���4��ͨ
void setConnectNumber(cv::InputArray src,
	cv::OutputArray dst, 
	int flag = LineTypes::LINE_4);

}//��namespace detail

#endif !_SOLUTION_MORPHOLOGY_TRANSFORM_H_