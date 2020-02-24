//�ṩ�й���������ӵ�ʵ��

#include"Solution/descriptor.h"
#include"Solution/base.h"
#include"Solution/grayscale_transfrom.h"

#ifdef __TEST__
//for test
#include"Solution/solution.h"
#endif

namespace detail {

//brief:��õ�����浥������������������ά��
//becare:���ͼ����ڵ������ڣ���ô�ͻ�������������ͬά�ȵ������
size_t HOGDescriptor::getDescriptorSize() const {
	return bins_ * (block_size_.width / cell_size_.width)* \
		(block_size_.height / cell_size_.height)* \
		((win_size_.height - block_size_.height) / block_stride_.height + 1)* \
		((win_size_.width - block_size_.width) / block_stride_.width + 1);
}

//brief:����ͼ���������
void HOGDescriptor::compute(cv::InputArray src, std::vector<float>& descriptors) {
	cv::Mat in = src.getMat();
	if (in.empty())
		return;

	//����window��
	//����block��

	//step1:���㵥�����ص��ݶȺͽǶ�
	cv::Mat dx, dy, grad, angle;
	detail::Sobel(in, dx, CV_32F, 1, 0, 3);//FIXME:���������sobel���
	detail::Sobel(in, dy, CV_32F, 0, 1, 3);
	cv::magnitude(dx, dy, grad);
	detail::cvArctan2(dy, dx, angle);

#ifdef __TEST__
	digital::__MatrixTest(&angle);
#endif

	////Ԥ�Ȱ���bins��Ŀ����ǰ�ֵ���ͬ������ȥ�������ظ�����
	//double unit = CV_PI / bins_;
	//auto set_bin = [unit](uint8_t* cursor) {
	//	float* cur = static_cast<float*>(static_cast<void*>(cursor));
	//  double val = *cur < 0 ? *cur + CV_PI : *cur;
	//	*cur = static_cast<float>(static_cast<int>(val / unit));
	//};
	//detail::grayscaleTransform(angle, set_bin);

	//Ԥ�ȷ�������������ռ�
	cv::Size win_number;//������Ҫ��������
	win_number.width = ((grad.cols - win_size_.width) / block_stride_.width +1);
	win_number.height = ((grad.rows - win_size_.height) / block_stride_.height +1);
	descriptors.resize(win_number.area() * getDescriptorSize());

	size_t angle_step = angle.rows * block_stride_.height;
	size_t grad_step = grad.rows * block_stride_.height;

	size_t cell_angle_step = angle.rows * cell_size_.height;
	size_t cell_grad_step = grad.rows * cell_size_.height;

	float* dst = &descriptors[0];
	
	//ָ��ĳһ�д��ڵ����Ͻ���������λ��
	float* iter_angle = angle.ptr<float>(0, 0);
	float* iter_grad = grad.ptr<float>(0, 0);

	for (int y = 0; y < win_number.height; ++y) {
		assert(iter_angle == angle.ptr<float>(y*block_stride_.height, 0));
		assert(iter_grad == grad.ptr<float>(y*block_stride_.height, 0));

		for (int x = 0; x < win_number.width; ++x) {
			//��һ�������ڲ��ڷֿ�
		    //ָ�򴰿ڵ����Ͻ�λ��
			float* tmp_angle = iter_angle + x * block_stride_.width;
			float* tmp_grad = iter_grad + x * block_stride_.width;

			//becare:���һ��block����������ֿռ�
			//for (int offset_y = 0; offset_y < win_size_.height; offset_y += block_stride_.height) {
			for (int offset_y = 0; offset_y < win_size_.height - block_stride_.height; offset_y += block_stride_.height) {
				//ָ��ǰblock���Ͻ�λ��
				//for test
				//auto dssdsds = y * block_stride_.height + offset_y;
				//auto sddddddddddxx = x * block_stride_.width;
				//auto sdddd = angle.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width);

				assert(tmp_angle == angle.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width));
				assert(tmp_grad == grad.ptr<float>(y*block_stride_.height+offset_y, x*block_stride_.width));

				float* block_angle = tmp_angle;
				float* block_grad = tmp_grad;

				//becare:���һ��block����������ֿռ�
				//for (int offset_x = 0; offset_x < win_size_.width; offset_x += block_stride_.width) {
				for (int offset_x = 0; offset_x < win_size_.width - block_stride_.width; offset_x += block_stride_.width) {
					//��һ��block�ڲ���cell
					//ÿ��cellͨ��ͳ��ֱ��ͼ�����Եõ�����bins�����ֵĵ��������
					//ÿ��cellû���ص����֣�block�����У�
					float* tmp_cell_angle = block_angle;
					float* tmp_cell_grad = block_grad;
					float* block_dst = dst;

					for (int cell_y = 0; cell_y < block_size_.height; cell_y += cell_size_.height) {
						//ָ��ǰcell�����Ͻ�λ��
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

							//FIXME:��cell�����ľ���͵���Ԫ�ػ������о�����ˡ�����
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

					//��ͬһ��block�ڲ���cell����L2 Norm
					//�ƶ���ͬһ�е���һ��block���Ͻ�λ��
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
		
		//�ƶ�����һ�д��������е����Ͻ�λ��
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

			//FIXME:����ֻ�����˵����������Բ�ֵ
			int index = static_cast<int>(A / UNIT);//����
			index = index >= bins_ ? bins_ - 1 : index;
			int other;
			double a = A - index * UNIT;//��index����Ȩ��

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