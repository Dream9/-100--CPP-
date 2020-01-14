#include"Solution/geometry_match.h"
#include"Solution/geometric_transform.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/base.h"
#include"Solution/type_extension.h"

#include<opencv2/highgui.hpp>

#include<ctime>
#include<random>
#include<map>

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

//brief:
class MatchTemplateFactory;
std::map<int, MatchTemplateFactory*> g_match_dict;
class MatchTemplateFactory{
public:
	MatchTemplateFactory() = default;

	static MatchTemplateFactory* create(int method) {
		auto iter = g_match_dict.find(method);
		if (iter == g_match_dict.end())
			return nullptr;
		return iter->second;
	}

	virtual void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) = 0;
};
//brief:���ڲ��ƽ����
class MatchSqdiff :public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchSqdiffNormed :public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:���ڻ����
class MatchCcorr:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchCcorrNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:���ڻ����ϵ��
class MatchCcoeff:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchCcoeffNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:����L1����(����ֵ)
//becare:�����ܲ�û�б�opencv��¼��matchTemplate,������Ϊһ�ֲο�����չ
class MatchAbsdiff:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchAbsdiffNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//��ʼ��ȫ�ֱ���
class GlobalDictInit : public digital::noncopyable {
public:
	GlobalDictInit() {
		g_match_dict.insert({ detail::TM_SQDIFF,new MatchSqdiff });
		g_match_dict.insert({ detail::TM_SQDIFF_NORMED,new MatchSqdiffNormed });

		g_match_dict.insert({ detail::TM_CCORR,new MatchCcorr});
		g_match_dict.insert({ detail::TM_CCORR_NORMED,new MatchCcorrNormed });

		g_match_dict.insert({ detail::TM_CCOEFF,new MatchCcoeff});
		g_match_dict.insert({ detail::TM_CCOEFF_NORMED,new MatchCcoeffNormed});

		g_match_dict.insert({ detail::TM_ABSDIFF,new MatchAbsdiff});
		g_match_dict.insert({ detail::TM_ABSDIFF_NORMED,new MatchAbsdiffNormed});
	}
};
GlobalDictInit __init_match_dict;//λ�������ռ��У��ⲿ���ɼ�

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

//brief��
void matchTemplate(cv::InputArray src, cv::InputArray templ, cv::OutputArray dst, int method) {
	assert(src.depth() == CV_8U || src.depth() == CV_32F);
	assert(templ.depth() == CV_8U || templ.depth() == CV_32F);

	cv::Mat in = src.getMat();
	cv::Mat kernel = templ.getMat();
	assert(kernel.cols <= in.cols && kernel.rows <= in.rows);

	//becare:�������ټ��Ĳ�����VALID����ĵط�
	dst.create(in.rows - kernel.rows + 1,in.cols - kernel.cols + 1, CV_32FC1);
	cv::Mat out = dst.getMat();

	auto ptr_implemnet = MatchTemplateFactory::create(method);
	if (!ptr_implemnet) {
		dealException(digital::kParameterNotMatch);
		return;
	}
	ptr_implemnet->operator()(in, kernel, out);
}

}//!namespace detail

namespace {

//brief:�÷����ı��ʾ�����L2������Ϊ��������ͼ�������Ե����ݣ����ֵԽСԽƥ��
void MatchSqdiff::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;

	auto squared_difference = [start, size, jump](uint8_t** arr, uint8_t* cur) {
		float sum = 0;
		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				//for test
				//digital::__printInfo("%u,%u\r\n",*iter,ptr[x]);
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L2ŷʽ�ռ����
				sum += tmp * tmp;
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = sum;
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:��MatchSqdiff����֮�ϣ������˹�һ�����Ӷ����ٳߴ��������
void MatchSqdiffNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;

	float square_sum = 0.;
	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//ģ��Ĺ�һ�������Ǻ㶨��

	auto squared_difference = [start, size, square_sum, jump](uint8_t** arr, uint8_t* cur) {
		double sum = 0.;
		float win_square_sum = 0.;
		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_square_sum += *iter * *iter;
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L2ŷʽ�ռ����
				sum += tmp * tmp;
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>(sum * std::pow(win_square_sum*square_sum, -0.5));//��һ��
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1), \
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:�÷����ı��ʾ��������������������Ϊ���������Եı�׼�����ֵԽ��Խƥ��
void MatchCcorr::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;

	auto squared_difference = [start, size, jump](uint8_t** arr, uint8_t* cur) {
		float sum = 0;
		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				sum += *iter++ * ptr[x];
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = sum;
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:��cross correlation�Ļ���֮�ϣ����й�һ�����������ٳߴ�������
void MatchCcorrNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	
	float square_sum = 0.;
	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//ģ��Ĺ�һ�������Ǻ㶨��

	auto squared_difference = [start, size, square_sum, jump](uint8_t** arr, uint8_t* cur) {
		double sum = 0;
		auto ptr = start;
		float win_square_sum = 0.;

		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_square_sum += *iter * *iter;
				sum += *iter++ * ptr[x];
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>(sum * std::pow(win_square_sum*square_sum,-0.5));//��һ��
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:�÷����ı����ǽ�����ͼ������ϵ����Ϊ���۱�׼��ֵԽ��Խƥ��
//becare:ʵ������û�н��й�һ����ʱ��ʵ�ʲ�����Э������Ϊ������׼�⣬��һ����Ϊ���ϵ��
//       ��ʱֵԽ�ӽ�1Խƥ�䣬Խ�ӽ�-1Խ��ƥ��
void MatchCcoeff::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	double total = static_cast<double>(templ.total());
	double total_1 = 1. / total;

	double templ_sum = 0.;
	auto get_mean= [&templ_sum](uint8_t* cursor) {
		templ_sum += *cursor;
	};
	detail::grayscaleTransform(templ, get_mean);//ģ��Ĺ�һ�������Ǻ㶨��
	double templ_mean = templ_sum * total_1;

	//brief:��һ���������ɾ�ֵ��Э�����ͳ��
	auto squared_difference = [start, size, templ_sum, templ_mean, total_1, total, jump]\
		(uint8_t** arr, uint8_t* cur) {
		double sum = 0.;
		double win_sum = 0.;
		double win_mean = 0.;
		auto ptr = start;

		//����ʽչ���󣬿���ͬʱͳ�ƾ�ֵ��Э����
		//double tmp_sum = 0.;
		//double tmp_mean = 0.;
		//for (int y = 0; y < size.height; ++y) {
		//	auto iter = arr[y];
		//	for (int x = 0; x < size.width; ++x) {
		//		tmp_sum+= *iter;
		//	}
		//}
		//tmp_mean = tmp_sum * total_1;
		//double res = 0.;
		//for (int y = 0; y < size.height; ++y) {
		//	auto iter = arr[y];
		//	for (int x = 0; x < size.width; ++x) {
		//		double ttt = (*iter - tmp_mean)*(ptr[x] - templ_mean);
		//		res += ttt;
		//		++iter;
		//	}
		//	ptr += jump;
		//}
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_sum += *iter;
				sum += *iter++ * (ptr[x] - templ_mean);
			}
			ptr += jump;
		}
		win_mean = win_sum * total_1;

		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>(sum + win_mean * templ_mean * total - win_mean * templ_sum);
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:�÷����ı����ǽ�����ͼ������ϵ����Ϊ���۱�׼��ֵԽ��Խƥ��
//becare:ʵ������û�н��й�һ����ʱ��ʵ�ʲ�����Э������Ϊ������׼�⣬��һ����Ϊ���ϵ��
//       ��ʱֵԽ�ӽ�1Խƥ�䣬Խ�ӽ�-1Խ��ƥ��
void MatchCcoeffNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	double total = static_cast<double>(templ.total());
	double total_1 = 1. / total;
	double templ_sum = 0.;
	double templ_sigma = 0.;
	double templ_mean;

	auto get_mean_sigma= [&templ_sum, &templ_sigma](uint8_t* cursor) {
		templ_sum += *cursor;
		templ_sigma += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_mean_sigma);//ģ��Ĺ�һ�������Ǻ㶨��
	templ_mean = templ_sum * total_1;
	templ_sigma = templ_sigma + templ_mean * templ_mean * total - 2 * templ_mean * templ_sum;

	//brief:��һ���������ɾ�ֵ��Э�����ͳ��
	auto squared_difference = [start, size, templ_sum, templ_mean, templ_sigma, total_1, total, jump]\
		(uint8_t** arr, uint8_t* cur) {
		double sum = 0.;
		double win_sum = 0.;
		double win_sigma = 0.;
		double win_mean = 0.;
		auto ptr = start;

		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_sum += *iter;
				win_sigma += *iter * *iter;
				sum += *iter++ * (ptr[x] - templ_mean);
			}
			ptr += jump;
		}
		win_mean = win_sum * total_1;
		win_sigma = win_sigma + win_mean * win_mean * total - 2 * win_mean * win_sum;
		double norm_factor = std::pow(win_sigma * templ_sigma, -0.5);

		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>((sum + win_mean * templ_mean * total - win_mean * templ_sum) * norm_factor);
			//std::pow((win_sigma + win_mean * total - 2 * win_mean * win_sum) * templ_sigma, -0.5));
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:�÷����ı��ʾ�����L1������Ϊ��������ͼ�������Ե����ݣ����ֵԽСԽƥ��
void MatchAbsdiff::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;

	auto squared_difference = [start, size, jump](uint8_t** arr, uint8_t* cur) {
		float sum = 0;
		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L1ŷʽ�ռ����
				sum += tmp;//��L2������Ψһ����
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = sum;
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:�÷����ı��ʾ�����L1������Ϊ��������ͼ�������Ե����ݣ����ֵԽСԽƥ��
void MatchAbsdiffNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	double square_sum = 0.;

	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//ģ��Ĺ�һ�������Ǻ㶨��

	auto squared_difference = [start, size, square_sum, jump](uint8_t** arr, uint8_t* cur) {
		float sum = 0;
		double win_square = 0.;

		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_square += *iter * *iter;
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L1ŷʽ�ռ����
				sum += tmp;//��L2������Ψһ����
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>(sum * std::pow(win_square * square_sum, -0.5));
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}



}//!namespace