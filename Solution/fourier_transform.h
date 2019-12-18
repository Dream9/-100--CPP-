//brief:���ļ��ṩ�й�Ƶ����任�йصĹ��ܷ�װ,����Ƶ�����˲�

#ifndef _SOLUTION_FOURIER_TRANSFORM_H_
#define _SOLUTION_FOURIER_TRANSFORM_H_

#include<opencv2/core.hpp>

namespace detail {

//brief:��opecv����
enum DftFlags {
    /** performs an inverse 1D or 2D transform instead of the default forward
        transform. */
    DFT_INVERSE        = 1,
    /** scales the result: divide it by the number of array elements. Normally, it is
        combined with DFT_INVERSE. */
    DFT_SCALE          = 2,
    /** performs a forward or inverse transform of every individual row of the input
        matrix; this flag enables you to transform multiple vectors simultaneously and can be used to
        decrease the overhead (which is sometimes several times larger than the processing itself) to
        perform 3D and higher-dimensional transformations and so forth.*/
    DFT_ROWS           = 4,
    /** performs a forward transformation of 1D or 2D real array; the result,
        though being a complex array, has complex-conjugate symmetry (*CCS*, see the function
        description below for details), and such an array can be packed into a real array of the same
        size as input, which is the fastest option and which is what the function does by default;
        however, you may wish to get a full complex array (for simpler spectrum analysis, and so on) -
        pass the flag to enable the function to produce a full-size complex output array. */
    DFT_COMPLEX_OUTPUT = 16,
    /** performs an inverse transformation of a 1D or 2D complex array; the
        result is normally a complex array of the same size, however, if the input array has
        conjugate-complex symmetry (for example, it is a result of forward transformation with
        DFT_COMPLEX_OUTPUT flag), the output is a real array; while the function itself does not
        check whether the input is symmetrical or not, you can pass the flag and then the function
        will assume the symmetry and produce the real output array (note that when the input is packed
        into a real array and inverse transformation is executed, the function treats the input as a
        packed complex-conjugate symmetrical array, and the output will also be a real array). */
    DFT_REAL_OUTPUT    = 32,
    /** specifies that input is complex input. If this flag is set, the input must have 2 channels.
        On the other hand, for backwards compatibility reason, if input has 2 channels, input is
        already considered complex. */
    DFT_COMPLEX_INPUT  = 64,
    /** performs an inverse 1D or 2D transform instead of the default forward transform. */
	//δʵ��
    //DCT_INVERSE        = DFT_INVERSE,
    /** performs a forward or inverse transform of every individual row of the input
        matrix. This flag enables you to transform multiple vectors simultaneously and can be used to
        decrease the overhead (which is sometimes several times larger than the processing itself) to
        perform 3D and higher-dimensional transforms and so forth.*/
    //DCT_ROWS           = DFT_ROWS
};

enum FrequencyFilterFlags {
	ILPF = 0x1,      //�����ͨ
	BLPF = 0x1 << 1, //������˹��ͨ
	GLPF = 0x1 << 2, //��˹��ͨ

	IHPF = 0x1 << 3, //�����ͨ
	BHPF = 0x1 << 4, //������˹��ͨ
	GHPF = 0x1 << 5, //��˹��ͨ

	//TODO:����ֻʵ���˲��ð�����˹�����Ĵ�ͨ���裩�˲�����������������չ
	BP = 0x1 << 6,  //������˹��ͨ
	BR = 0x1 << 7,  //������˹����

	//TODO:����������ʽ���ݲ�������Ŀǰ���õ��������˲�������������
	NOTCH = 0x1 << 8,
};

//brief:����Ҷ�任�Լ����任
//parameter:src:��������
//          dst:Ŀ��
//          flags:������������������ƣ��μ�opeccv::dft
void dft(cv::Mat& src, 
	cv::Mat& dst,
	int flags = 0);

//brief:����Ҷ���任
//parameter:�μ�opencv::idft
void idft(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0);

//brief:���ٸ���Ҷ�任
//parameter:�μ�opencv::dft��
//becare:��������Ƿ�װ��getOptimalDFTSize��Ȼ��ת��opencv�ӿ�
//       �û���Ҫ���н�ȡ���ಿ������
void fft(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0);

//brief:�Ӹ���Ҷ�任����еõ������ף�����Ҷ�ף�
void getAmplitudeSpectrum(cv::Mat& src, cv::Mat& dst);

//brief::�Ӹ���Ҷ�任�������ȡ��λ�ף�����ף�
void getPhaseSpectrum(cv::Mat& src, cv::Mat& dst);

//brief:���층��Ҷ�׵ĻҶȼ���������ʾ,Ĭ�Ϸ���CV_8U��ȵĽ��
cv::Mat grayscaleAmplitudeSpctrum(cv::Mat& spectrum);

//brief:�õ���Ƶ����λ�������ͼ��ԭ����exp[(2*PI*x(M/2)*x/M)i] == pow(-1.,x),������
//     Ƶ���ƶ�M/2,2D����1D
cv::Mat centralize(cv::Mat& src);

//brief:Ƶ�����˲���
//paramter:src,�ȴ��任��Ƶ����ͼ��������Ҫ�����������ͨ����������
//         dst:���ݴ洢�ص�
//         flags:ִ�еĲ���
//         data:�ṩ��ͬ������Ҫ����Ч����
void frequencyDomainFilter(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0,
	void* data = nullptr);

}



#endif // _SOLUTION_FOURIER_TRANSFORM_H_
