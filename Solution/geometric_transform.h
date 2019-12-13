//brief:提供有关几何变换的功能封装

#ifndef _SOLUTION_GEOMETRIC_TRANSFROM_H_
#define _SOLUTION_GEOMETRIC_TRANSFROM_H_

#include<opencv2/imgproc.hpp>

#include<functional>

namespace detail {

//brief:插值方式,与opencv兼容
enum InterpolationFlags{
    /** nearest neighbor interpolation */
    INTER_NEAREST        = 0,
    /** bilinear interpolation */
    INTER_LINEAR         = 1,
    /** bicubic interpolation */
    INTER_CUBIC          = 2,
    /** resampling using pixel area relation. It may be a preferred method for image decimation, as
    it gives moire'-free results. But when the image is zoomed, it is similar to the INTER_NEAREST
    method. */
    INTER_AREA           = 3,
    /** Lanczos interpolation over 8x8 neighborhood */
    INTER_LANCZOS4       = 4,
    /** Bit exact bilinear interpolation */
    INTER_LINEAR_EXACT = 5,
    /** mask for interpolation codes */
    INTER_MAX            = 7,
    /** flag, fills all of the destination image pixels. If some of them correspond to outliers in the
    source image, they are set to zero */
    WARP_FILL_OUTLIERS   = 8,
    /** flag, inverse transformation

    For example, #linearPolar or #logPolar transforms:
    - flag is __not__ set: \f$dst( \rho , \phi ) = src(x,y)\f$
    - flag is set: \f$dst(x,y) = src( \rho , \phi )\f$
    */
    WARP_INVERSE_MAP     = 16
};

static const double kPI = 3.141592653;

typedef std::function<void(int, int, uint8_t*)> TriversalOperatorType;

//brief:rad/deg转换
inline double Rad2Deg(double rad) {
	return rad * 180 / kPI;
}
inline double Deg2Rad(double deg) {
	return deg * kPI / 180;
}

//brief:计算旋转缩放的仿射变换参数，参见cv::getRotationMatrix2D
//parameter:rotation_center:旋转中心
//          angle_count_clock:按逆时针方向计算出来的旋转角度，顺时针取反即可
//          x_scale:x方向缩放比例
//          y_scale:y方向缩放比例，默认与x_scale一致
//return:仿射矩阵，类型为CV_64F,大小为3*3，这里核opencv略有不同，源码返回的是2*3，即最后补齐的一行被忽略了
cv::Mat getRotationMatrix2D(const cv::Point& rotation_center,
	double angle_count_clock, 
	double x_scale, 
	double y_scale = -1);

//brief:获得经过变换后图像的大小(正好完整显示原始数据)
//parameter:M:变换矩阵
//          original_size:原始数据大小
//          xmin/ymin如非nullptr,则记录变换后的x/y方向最小值，（可以用于偏移图像，使之定位到原点）
cv::Size getSizeAfterWarpAffine(const cv::Mat& M,
	const cv::Size& original_size,
	double* xmin = nullptr,
	double* ymin = nullptr);

//brief:对图像实施仿射变换，参见cv::wrapAffine
//parameter:src:输入图像
//          dst:输出图像，与src具有相同类型
//          M:变换矩阵
//          size:输出图像尺寸
//          interpolate_type:插值类型
//          constant_value:对于变换后超出图像映射范围的默认值
//          beed_complete_include:若为true,则会重新调整M和size,使得变换后的有效结果恰好完整位于dst之中
//                               这种调整只会调整平移量，旋转缩放不会改变，用户也可以在外部提前完成此变换
void warpAffine(const cv::Mat& src,
	cv::Mat& dst,
	cv::Mat& M,
	cv::Size size,
	int interpolate_type = detail::INTER_LINEAR,
	const cv::Scalar& constant_value = cv::Scalar(),
	bool need_complete_include = false);

//brief:对图像遍历的封装，在对应位置调用用户传入函数，本函数亦作为其他几何变换的基础操作
//parameter:src:输入图像
//          op:在src的[x,y]位置进行用户指定的操作
//becare: TriversalOperatorType的参数类型分别对应x,y坐标以及该点数据的handler
void geometricTriversal(cv::Mat& src, const TriversalOperatorType& op);


}//!namespace detail


#endif // !_SOLUTION_GEOMETRIC_TRANSFROM_H_
