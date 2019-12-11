//brief:�ṩ�йؼ��α任�Ĺ��ܷ�װ

#ifndef _SOLUTION_GEOMETRIC_TRANSFROM_H_
#define _SOLUTION_GEOMETRIC_TRANSFROM_H_

#include<opencv2/imgproc.hpp>

#include<functional>

namespace detail {

//brief:��ֵ��ʽ,��opencv����
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

//brief:rad/degת��
inline double Rad2Deg(double rad) {
	return rad * 180 / kPI;
}
inline double Deg2Rad(double deg) {
	return deg * kPI / 180;
}

//brief:������ת���ŵķ���任�������μ�cv::getRotationMatrix2D
//parameter:rotation_center:��ת����
//          angle_count_clock:����ʱ�뷽������������ת�Ƕȣ�˳ʱ��ȡ������
//          x_scale:x�������ű���
//          y_scale:y�������ű�����Ĭ����x_scaleһ��
//return:�����������ΪCV_64F,��СΪ3*3�������opencv���в�ͬ��Դ�뷵�ص���2*3����������һ�б�������
cv::Mat getRotationMatrix2D(const cv::Point& rotation_center,
	double angle_count_clock, 
	double x_scale, 
	double y_scale = -1);

//brief:��ͼ��ʵʩ����任���μ�cv::wrapAffine
//parameter:src:����ͼ��
//          dst:���ͼ����src������ͬ����
//          M:�任����
//          size:���ͼ��ߴ�
//          interpolate_type:��ֵ����
//          constant_value:���ڱ任�󳬳�ͼ��ӳ�䷶Χ��Ĭ��ֵ
void warpAffine(const cv::Mat& src,
	cv::Mat& dst,
	const cv::Mat& M,
	cv::Size size,
	int interpolate_type = detail::INTER_LINEAR,
	const cv::Scalar& constant_value = cv::Scalar());

//brief:��ͼ������ķ�װ���ڶ�Ӧλ�õ����û����뺯��������������Ϊ�������α任�Ļ�������
//parameter:src:����ͼ��
//          op:��src��[x,y]λ�ý����û�ָ���Ĳ���
//becare: TriversalOperatorType�Ĳ������ͷֱ��Ӧx,y�����Լ��õ����ݵ�handler
void geometricTriversal(cv::Mat& src, const TriversalOperatorType& op);


}//!namespace detail


#endif // !_SOLUTION_GEOMETRIC_TRANSFROM_H_
