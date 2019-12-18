//brief:本文件提供有关频率域变换有关的功能封装,包括频率域滤波

#ifndef _SOLUTION_FOURIER_TRANSFORM_H_
#define _SOLUTION_FOURIER_TRANSFORM_H_

#include<opencv2/core.hpp>

namespace detail {

//brief:与opecv兼容
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
	//未实现
    //DCT_INVERSE        = DFT_INVERSE,
    /** performs a forward or inverse transform of every individual row of the input
        matrix. This flag enables you to transform multiple vectors simultaneously and can be used to
        decrease the overhead (which is sometimes several times larger than the processing itself) to
        perform 3D and higher-dimensional transforms and so forth.*/
    //DCT_ROWS           = DFT_ROWS
};

enum FrequencyFilterFlags {
	ILPF = 0x1,      //理想低通
	BLPF = 0x1 << 1, //巴特沃斯低通
	GLPF = 0x1 << 2, //高斯低通

	IHPF = 0x1 << 3, //理想高通
	BHPF = 0x1 << 4, //巴特沃斯高通
	GHPF = 0x1 << 5, //高斯高通

	//TODO:这里只实现了采用巴特沃斯函数的带通（阻）滤波器，增加其他的扩展
	BP = 0x1 << 6,  //巴特沃斯带通
	BR = 0x1 << 7,  //巴特沃斯带阻

	//TODO:采用其他形式的陷波函数，目前采用的是理想滤波器，存在振铃
	NOTCH = 0x1 << 8,
};

//brief:傅里叶变换以及反变换
//parameter:src:输入数据
//          dst:目标
//          flags:处理方法和输入输出控制，参见opeccv::dft
void dft(cv::Mat& src, 
	cv::Mat& dst,
	int flags = 0);

//brief:傅里叶反变换
//parameter:参见opencv::idft
void idft(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0);

//brief:快速傅里叶变换
//parameter:参见opencv::dft，
//becare:这里仅仅是封装了getOptimalDFTSize，然后转调opencv接口
//       用户需要自行截取多余部分数据
void fft(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0);

//brief:从傅里叶变换结果中得到幅度谱（傅里叶谱）
void getAmplitudeSpectrum(cv::Mat& src, cv::Mat& dst);

//brief::从傅里叶变换结果中提取相位谱（相角谱）
void getPhaseSpectrum(cv::Mat& src, cv::Mat& dst);

//brief:拉伸傅里叶谱的灰度级，便于显示,默认返回CV_8U深度的结果
cv::Mat grayscaleAmplitudeSpctrum(cv::Mat& spectrum);

//brief:得到零频中心位于中央的图像，原理是exp[(2*PI*x(M/2)*x/M)i] == pow(-1.,x),可以让
//     频谱移动M/2,2D类似1D
cv::Mat centralize(cv::Mat& src);

//brief:频率域滤波器
//paramter:src,等待变换的频率域图像描述，要求必须有两个通带，代表复数
//         dst:数据存储地点
//         flags:执行的操作
//         data:提供不同操作需要的有效数据
void frequencyDomainFilter(cv::Mat& src,
	cv::Mat& dst,
	int flags = 0,
	void* data = nullptr);

}



#endif // _SOLUTION_FOURIER_TRANSFORM_H_
