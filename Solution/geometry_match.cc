#include"Solution/geometry_match.h"
#include"Solution/geometric_transform.h"
#include"Solution/type_extension.h"

#include<opencv2/highgui.hpp>

#include<ctime>
#include<random>

namespace {

const int kUnvisited = -1;
const int kVisited = 0;
const int kDeleted = -4;

//brief:����״̬����
void __test_mask(cv::InputArray src) {
	cv::Mat in = src.getMat();
	cv::Mat dsds;
	cv::threshold(in, dsds, 1, 255, CV_THRESH_BINARY);
	cv::imshow("test mask state", dsds);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:���Ի���ռ�
void __show_hough_space(cv::InputArray src) {
	cv::Mat accumulator = src.getMat().clone();
	cv::convertScaleAbs(accumulator, accumulator, 1, 1);
	string name = "HoughSpace";
	cv::namedWindow(name, CV_WINDOW_NORMAL);
	cv::Mat accu_img;
	cv::normalize(accumulator, accu_img, 1, UINT8_MAX, cv::NORM_MINMAX, CV_8U);
	cv::imshow(name, accu_img);
	cv::waitKey(0);
	cv::destroyWindow(name);
}

}//!namespace

namespace detail {

//brief:�����׼ֱ�߼��
//beacre:���ﴢ����ֻ����CV_64C2���͵�������������ʹ��vector<Vec2f>��¼
//       �������ݱ�����CV_8UC1�Ķ�ֵͼ�񣬰����е�ֵΪUINT8_MAX��Ϊǰ������
void HoughLines(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold) {
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//��ʼ���ۼ���
	//becare:�����opencv����һ���Ĳ��죬��Ϊ�и�����������ʼ������
	cv::Size size = in.size();
	double max_distance = std::sqrt(size.height*size.height + size.width*size.width);
	int angle_number = static_cast<int>(std::ceil(CV_PI / theta));
	int rho_number = static_cast<int>(2 * max_distance / rho) + 4;
	double half_rho = rho * 0.5;
	double offset = rho_number * rho * 0.5 - half_rho;
	cv::Mat accumulator = cv::Mat::zeros(cv::Size(rho_number, angle_number), CV_32SC1);

	//���б�׼����任
	auto iter = in.data;
	for (int y = 0; y < size.height; ++y) {
		for (int x = 0; x < size.width; ++x) {
			if (*iter++ != UINT8_MAX)
				continue;
			double alpha = 0;
			for (int i = 0; i < angle_number; ++i) {
				double rho_current = x * std::cos(alpha) + y * std::sin(alpha);
				int pos = static_cast<int>((rho_current + offset) / rho);

				accumulator.at<int32_t>(i, pos) += 1;
				alpha += theta;
			}

		}
	}

#ifdef SHOW_PROCESS
	__show_hough_space(accumulator);
#endif

	//ͳ��̽���ֱ��
	std::vector<cv::Vec2d> tmp;
	auto cur = accumulator.ptr<int>(0, 0);
	size = accumulator.size();
	double alpha = 0;
	for (int y = 0; y < size.height; ++y) {
		double distance = - offset + half_rho;
		for (int x = 0; x < size.width; ++x) {
			//if(fabs(distance +0.5*rho -232)<1e-5 && fabs(alpha-1.51844)<1e-5)
			//if (*cur++ <= threshold) {//FIXME:==ʱ��Ӧ������
			if (*cur++ < threshold) {
				distance += rho;
				continue;
			}

			//�ڻ���ֱ�߿ռ��г�����ֵ
			//tmp.emplace_back(alpha, distance);//Ϊ����opecv���ݣ��������Ⱦ����Ƕ�
			tmp.emplace_back(distance, alpha);
			distance += rho;
		}
		alpha += theta;
	}

	//�������䵽�û��ռ�
	int len = static_cast<int>(tmp.size());
	lines.create(cv::Size(1, len), CV_64FC2);
	cv::Mat out = lines.getMat();
	auto ptr = out.ptr<double>(0, 0);
	for (auto& val : tmp) {
		*ptr++ = val[0];
		*ptr++ = val[1];
	}
}

#define GET_DISTANCE2D(p1, p2) (std::sqrt(std::pow(abs(p1.x-p2.x),2)+std::pow(abs(p1.y-p2.y),2)))
//brief:���ʻ���ֱ�߼��
//becare:�Ѿ����� /opencv3.4.1/opencv/sources/modules/imgproc/src/hough.cpp������
void HoughLinesP(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold,
	double min_line_length, double max_gap_length){
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//��ȡ��ѡ�㣬����ʼ�����루״̬������
	cv::Mat _state = cv::Mat::zeros(in.size(), CV_8UC1);
	uint8_t* state = _state.data;
	size_t step = _state.step;

	cv::Size size = in.size();
	std::vector<std::pair<int, int>>candidate;
	auto get_candi = [&](int x, int y, uint8_t* cursor) {
		if (*cursor != UINT8_MAX)
			return;
		state[y*step + x] = UINT8_MAX;//���Ѷ�Ӧλ�õ�_state��ǿ���
		candidate.emplace_back(x,y);
	};
	detail::geometricTriversal(in, get_candi);

	//��ʼ���ۼ���
	double max_distance = std::sqrt(size.height*size.height + size.width*size.width);
	int angle_number = static_cast<int>(std::ceil(CV_PI / theta));
	int rho_number = static_cast<int>(2 * max_distance / rho) + 1;
	double offset = rho_number * rho * 0.5;
	cv::Mat accumulator = cv::Mat::zeros(cv::Size(rho_number, angle_number), CV_32SC1);

	int len = static_cast<int>(candidate.size());
	int unvisited_end = len;
	min_line_length = min_line_length == -1 ? 0 : min_line_length;
	int max_line_gap = max_gap_length == -1 ? INT_MAX : static_cast<int>(max_gap_length);

	srand(unsigned int (std::time(0)));
	std::vector<cv::Vec4i> vec;
	//�������µĵ�
	while (unvisited_end > 0) {
		int pos = rand()%unvisited_end;
		int x = candidate[pos].first;
		int y = candidate[pos].second;
		std::swap(candidate[pos], candidate[--unvisited_end]);//�����ӳ�ɾ���ķ�ʽ

		if (!state[y * step + x])//�Ѿ������������������ֱ��
			continue;

		//�ۼӼ������������Ѿ����������ĵ�ɾ�����ӳ٣�
		//FIXME:ÿһ��ֻȡֵ���Ľ��д���֮ǰ�����ˣ�����
		int max_vote = 0;
		//double max_rho;//FIXME:��������ʵ�ֲ�����Ҫ��¼���룡��
		double max_angle;
		double alpha = 0;
		for (int i = 0; i < angle_number; ++i) {
			double cos_a = std::cos(alpha);
			double sin_a = std::sin(alpha);
			double rho_current = x * cos_a + y * sin_a;
			int pos = static_cast<int>((rho_current + offset) / rho);
			int32_t cur = accumulator.at<int32_t>(i, pos) += 1;

			if (cur > max_vote) {
				max_vote = cur;
				max_angle = alpha;
			}
			alpha += theta;
		}

		if (max_vote < threshold)
			continue;

		//������������Ѱ�Ҽ�Ͼ���С��max_gap_distance���߶�
		double cos_a = std::cos(max_angle);
		double sin_a = std::sin(max_angle);
		double dx0, dy0;
		if (fabs(cos_a) < fabs(sin_a)) {
			//x���������Ŀ�
			dx0 = sin_a < 0 ? -1 : 1;
			dy0 = fabs(sin_a) < digital::k_EPS ? 0 : dx0 * cos_a / sin_a;
		}
		else {
			//y���������Ŀ�
			dy0 = cos_a > 0 ? 1 : -1;
			dx0 = fabs(cos_a) < digital::k_EPS ? 0 : dy0 * sin_a / cos_a;
		}

		cv::Point line_end[2] = { {0,0},{0,0} };
		for (int k = 0; k < 2; ++k) {
			if (k == 1) {
				dx0 = -dx0;
				dy0 = -dy0;
			}
			int gap = 0;
			double x0 = x, y0 = y;
			for (;; x0 += dx0, y0 += dy0) {
				int i = static_cast<int>(x0);
				int j = static_cast<int>(y0);

				if (i<0 || i > size.width ||
					j<0 || j > size.height) {
					break;
				}
				auto mask = state + j * step + i;
				if (*mask) {
					gap = 0;
					line_end[k].x = i;
					line_end[k].y = j;
				}
				else if (++gap > max_line_gap)
					break;
			}
		}
		//�ж��߶γ����Ƿ���������
		//bool good_line = GET_DISTANCE2D(line_end[0], line_end[1]) >= min_line_length;//opencv�в�����ֱ���ж�delta_x delta_y�ķ�ʽ
		bool good_line = (abs(line_end[0].x - line_end[1].x) >= min_line_length ||
			abs(line_end[0].y - line_end[1].y) >= min_line_length);
		dx0 = -dx0, dy0 = -dy0;
		//�ڶ�������ֱ�߷��������Ŀ�����������Щ��Ŀ�����
		//ע�������˼�룬���ۺû������Щ�㶼���ڲ�����㣬
		//������Ȼ�������У���Ҳ���Ǳ��㷨���ԭ��֮һ
		for (int k = 0; k < 2; ++k) {
			if (k == 1) {
				dx0 = -dx0;
				dy0 = -dy0;
			}
			double x0 = x, y0 = y;
			for (;; x0 += dx0, y0 += dy0) {
				int i = static_cast<int>(x0);
				int j = static_cast<int>(y0);

				if (i<0 || i > size.width ||
					j<0 || j > size.height) {
					break;
				}
				auto mask = state + j * step + i;
				if (*mask) {
					*mask = 0;//ʹ���䲻����
					if (!good_line)
						continue;

					//�����Щ����Ϊ�߶α�������ˣ���ô���ǶԻ���
					//�ռ�Ĺ��ױ���ȫ������
					//ע������������У�������δ���׵��˶�ɾ��һ��
					double alpha = 0;
					for (int i = 0; i < angle_number; ++i) {
						double cos_a = std::cos(alpha);
						double sin_a = std::sin(alpha);
						double rho_current = i * cos_a + j * sin_a;
						int pos = static_cast<int>((rho_current + offset) / rho);
						accumulator.at<int32_t>(i, pos) -= 1;
						alpha += theta;
					}
				}
				//��ʱ���˳������ǣ��Ѹ��߶θ����꼴��
				if (i == line_end[k].x && j == line_end[k].y)
					break;
			}
		}

		if (good_line) {
			//��Ҫ��¼������ռ���
			vec.emplace_back(line_end[0].x, line_end[0].y,
				line_end[1].x, line_end[1].y);
			//for test
			//__test_mask(_state);
		}
	}

	//����������û��ռ�
	len = static_cast<int>(vec.size());
	lines.create(cv::Size(1, len), CV_32SC4);
	cv::Mat out = lines.getMat();
	auto ptr = out.ptr<int>(0, 0);
	for (auto& val : vec) {
		*ptr++ = val[0];
		*ptr++ = val[1];
		*ptr++ = val[2];
		*ptr++ = val[3];
	}
}
#undef GET_DISTANCE2D


//brief:��̽���ֱ�ߵ��ӵ�ͼ����
void overlapHoughImage(cv::InputArray origin, cv::InputArray mask, cv::InputArray hough, 
	cv::OutputArray dst, cv::Scalar val) {
	cv::Mat original = origin.getMat();
	cv::Mat lines = hough.getMat();
	cv::Mat binary = mask.getMat();

	assert(original.depth() == CV_8U);
	assert(lines.type() == CV_64FC2);
	assert(binary.type() == CV_8UC1);
	assert(lines.isContinuous());

	dst.create(original.size(), original.type());
	cv::Mat img = dst.getMat();

	memcpy(img.data, original.data, original.total()*original.elemSize());

	cv::Vec2d* iter = lines.ptr < cv::Vec2d>(0, 0);
	size_t len = lines.total();
	for (size_t i = 0; i < len; i++)
	{
		double rho = iter[i][0], theta = iter[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		cv::line(img, pt1, pt2, val, 3, cv::LINE_AA);
	}

	//���ֵͼ����һ��������
	cv::Size size = original.size();
	auto it = img.data;
	auto cur = original.data;
	auto ptr = binary.data;
	auto elemsize = original.elemSize();
	auto channels = original.channels();

	for(int y =0; y < size.height; ++y){
		for (int x = 0; x < size.width; ++x) {

			assert(it == img.ptr(y, x));
			assert(cur == original.ptr(y, x));
			assert(ptr == binary.ptr(y, x));
			if (*ptr == 0) {//���зǽ�������ȡԭͼ���ֵ���Ӷ�ʹ��ֱ�������ڶ�ֵͼ��
				for (int c = 0; c < channels; ++c) 
					it[c] = cur[c];
			}
			it += elemsize;
			cur += elemsize;
			++ptr;
		}
	}
}


}//!namespace detail