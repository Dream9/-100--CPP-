//提供有关特征描绘子的实现

#include"Solution/descriptor.h"
#include"Solution/base.h"
#include"Solution/grayscale_transfrom.h"

#ifdef __TEST__
//for test
#include"Solution/solution.h"
#endif

namespace detail {

//brief:获得单个描绘单个窗口特征的描述子维度
//becare:如果图像大于单个窗口，那么就会产生多个具有相同维度的描绘子
size_t HOGDescriptor::getDescriptorSize() const {
	return bins_ * (block_size_.width / cell_size_.width)* \
		(block_size_.height / cell_size_.height)* \
		((win_size_.height - block_size_.height) / block_stride_.height + 1)* \
		((win_size_.width - block_size_.width) / block_stride_.width + 1);
}

//brief:计算图像的描述子
void HOGDescriptor::compute(cv::InputArray src, std::vector<float>& descriptors) {
	cv::Mat in = src.getMat();
	if (in.empty())
		return;

	//单个window内
	//单个block内

	//step1:计算单个像素的梯度和角度
	cv::Mat dx, dy, grad, angle;
	detail::Sobel(in, dx, CV_32F, 1, 0, 3);//FIXME:这里采用了sobel差分
	detail::Sobel(in, dy, CV_32F, 0, 1, 3);
	cv::magnitude(dx, dy, grad);
	detail::cvArctan2(dy, dx, angle);

#ifdef __TEST__
	digital::__MatrixTest(&angle);
#endif

	////预先按照bins数目，提前分到不同格子中去，减少重复计算
	//double unit = CV_PI / bins_;
	//auto set_bin = [unit](uint8_t* cursor) {
	//	float* cur = static_cast<float*>(static_cast<void*>(cursor));
	//  double val = *cur < 0 ? *cur + CV_PI : *cur;
	//	*cur = static_cast<float>(static_cast<int>(val / unit));
	//};
	//detail::grayscaleTransform(angle, set_bin);

	//预先分配描述子所需空间
	cv::Size win_number;//代表需要几个窗口
	win_number.width = ((grad.cols - win_size_.width) / block_stride_.width +1);
	win_number.height = ((grad.rows - win_size_.height) / block_stride_.height +1);
	descriptors.resize(win_number.area() * getDescriptorSize());

	size_t angle_step = angle.rows * block_stride_.height;
	size_t grad_step = grad.rows * block_stride_.height;

	size_t cell_angle_step = angle.rows * cell_size_.height;
	size_t cell_grad_step = grad.rows * cell_size_.height;

	float* dst = &descriptors[0];
	
	//指向某一行窗口的左上角所在行首位置
	float* iter_angle = angle.ptr<float>(0, 0);
	float* iter_grad = grad.ptr<float>(0, 0);

	for (int y = 0; y < win_number.height; ++y) {
		assert(iter_angle == angle.ptr<float>(y*block_stride_.height, 0));
		assert(iter_grad == grad.ptr<float>(y*block_stride_.height, 0));

		for (int x = 0; x < win_number.width; ++x) {
			//对一个窗口内部在分块
		    //指向窗口的左上角位置
			float* tmp_angle = iter_angle + x * block_stride_.width;
			float* tmp_grad = iter_grad + x * block_stride_.width;

			//becare:最后一个block必须许留充分空间
			//for (int offset_y = 0; offset_y < win_size_.height; offset_y += block_stride_.height) {
			for (int offset_y = 0; offset_y < win_size_.height - block_stride_.height; offset_y += block_stride_.height) {
				//指向当前block左上角位置
				//for test
				//auto dssdsds = y * block_stride_.height + offset_y;
				//auto sddddddddddxx = x * block_stride_.width;
				//auto sdddd = angle.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width);

				assert(tmp_angle == angle.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width));
				assert(tmp_grad == grad.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width));

				float* block_angle = tmp_angle;
				float* block_grad = tmp_grad;

				//becare:最后一个block必须许留充分空间
				//for (int offset_x = 0; offset_x < win_size_.width; offset_x += block_stride_.width) {
				for (int offset_x = 0; offset_x < win_size_.width - block_stride_.width; offset_x += block_stride_.width) {
					//对一个block内部分cell
					//每个cell通过统计直方图，可以得到含有bins个数字的单个描绘子
					//每个cell没有重叠部分（block可能有）
					float* tmp_cell_angle = block_angle;
					float* tmp_cell_grad = block_grad;
					float* block_dst = dst;

					for (int cell_y = 0; cell_y < block_size_.height; cell_y += cell_size_.height) {
						//指向当前cell的左上角位置
						float* cell_angle = tmp_cell_angle;
						float* cell_grad = tmp_cell_grad;
						for (int cell_x = 0; cell_x < block_size_.width; cell_x += cell_size_.width) {
							//for test
							//auto dss = y * block_stride_.height + offset_y + cell_y;
							//auto sdsd = x * block_stride_.width + offset_x + cell_x;
							//auto sds =(angle.ptr<float>(y*block_stride_.height+offset_y+cell_y,\
							//	x*block_stride_.width+offset_x+cell_x));

							assert(cell_angle == angle.ptr<float>(y*block_stride_.height+offset_y+cell_y,\
								x*block_stride_.width+offset_x+cell_x));
							assert(cell_grad == grad.ptr<float>(y*block_stride_.height+offset_y+cell_y,\
								x*block_stride_.width+offset_x+cell_x));

							//FIXME:把cell跳过的距离和单个元素换到下行距离混了。。。
							//_calc_cell_descriptor(cell_angle, cell_grad, cell_angle_step, cell_grad_step, dst);
							_calc_cell_descriptor(cell_angle, cell_grad, angle.rows, grad.rows, dst);
							//for test
							//_calc_cell_descriptor(cell_angle, cell_grad, angle.rows,\
							//	grad.rows, dst, &angle, dss,sdsd);
							
							dst += bins_;

							cell_angle += cell_size_.width;
							cell_grad += cell_size_.width;
						}
						tmp_cell_angle += cell_angle_step;
						tmp_cell_grad += cell_grad_step;
					}

					//对同一个block内部的cell进行L2 Norm
					//移动到同一行的下一个block左上角位置
					double l2sum = 0.0;
					double eps = 1e-6;
					std::for_each(block_dst, dst, [&l2sum](float val) {
						l2sum += val * val;
					});
					l2sum = sqrt(l2sum);
					std::transform(block_dst, dst, block_dst, [l2sum,eps](float val) {
						return static_cast<float>(val / (l2sum + eps));
					});

					block_angle += block_stride_.width;
					block_grad += block_stride_.width;
				}
				tmp_angle += angle_step;
				tmp_grad += grad_step;
			}
		}
		
		//移动到下一行窗口所在行的左上角位置
		iter_angle += angle_step;
		iter_grad += grad_step;
	}
}

//brief:
void HOGDescriptor::_calc_cell_descriptor(float* angle, float* grad, size_t angle_step,
	size_t grad_step, float* dst, cv::Mat* test,int row,int col) {
	double UNIT = CV_PI / bins_;
	double UNIT_HALF = UNIT / 2;
	double PI2 = CV_PI * 2;

	for (int y = 0; y < cell_size_.height; ++y) {
		float* cur_angle = angle;
		float* cur_grad = grad;

		for (int x = 0; x < cell_size_.width; ++x) {
			//for test
			//assert(cur_angle == test->ptr<float>(row + y, col + x));

			double A = *cur_angle;
			double G = *cur_grad;
			A = A < 0 ? A + CV_PI : A;

			//FIXME:这里只采用了单变量的线性插值
			int index = static_cast<int>(A / UNIT);//索引
			index = index >= bins_ ? bins_ - 1 : index;
			int other;
			double a = A - index * UNIT;//到index处的权重

			if (a < UNIT_HALF) {
				other = index == 0 ? bins_ - 1 : index - 1;
				a = UNIT_HALF - a;
			}
			else {
				other = index == bins_ - 1 ? 0 : index + 1;
				a = a - UNIT_HALF;
			}

			double val = G * a / UNIT;
			dst[index] += static_cast<float>(val);
			dst[other] += static_cast<float>(G - val);
			++cur_angle;
			++cur_grad;
		}
		angle += angle_step;
		grad += grad_step;
	}
}

}//!namespace detail