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

//brief:测试状态掩码
void __test_mask(cv::InputArray src) {
	cv::Mat in = src.getMat();
	cv::Mat dsds;
	cv::threshold(in, dsds, 1, 255, CV_THRESH_BINARY);
	cv::imshow("test mask state", dsds);
	cv::waitKey(0);
	cv::destroyAllWindows();
}

//brief:测试霍夫空间
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
//brief:基于差的平方和
class MatchSqdiff :public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchSqdiffNormed :public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:基于互相关
class MatchCcorr:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchCcorrNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:基于互相关系数
class MatchCcoeff:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchCcoeffNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//brief:基于L1范数(绝对值)
//becare:本功能并没有被opencv收录到matchTemplate,可以作为一种参考与扩展
class MatchAbsdiff:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
class MatchAbsdiffNormed:public MatchTemplateFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst)override;
};
//初始化全局变量
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
GlobalDictInit __init_match_dict;//位于匿名空间中，外部不可见

}//!namespace

namespace detail {

//brief:霍夫标准直线检测
//beacre:这里储存结果只返回CV_64C2类型的行向量，可以使用vector<Vec2f>记录
//       输入数据必须是CV_8UC1的二值图像，把其中的值为UINT8_MAX作为前景数据
void HoughLines(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold) {
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//初始化累加器
	//becare:这里和opencv存在一定的差异，因为切割距离的坐标起始点问题
	cv::Size size = in.size();
	double max_distance = std::sqrt(size.height*size.height + size.width*size.width);
	int angle_number = static_cast<int>(std::ceil(CV_PI / theta));
	int rho_number = static_cast<int>(2 * max_distance / rho) + 4;
	double half_rho = rho * 0.5;
	double offset = rho_number * rho * 0.5 - half_rho;
	cv::Mat accumulator = cv::Mat::zeros(cv::Size(rho_number, angle_number), CV_32SC1);

	//进行标准霍夫变换
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

	//统计探测的直线
	std::vector<cv::Vec2d> tmp;
	auto cur = accumulator.ptr<int>(0, 0);
	size = accumulator.size();
	double alpha = 0;
	for (int y = 0; y < size.height; ++y) {
		double distance = - offset + half_rho;
		for (int x = 0; x < size.width; ++x) {
			//if(fabs(distance +0.5*rho -232)<1e-5 && fabs(alpha-1.51844)<1e-5)
			//if (*cur++ <= threshold) {//FIXME:==时不应被过滤
			if (*cur++ < threshold) {
				distance += rho;
				continue;
			}

			//在霍夫直线空间中超过阈值
			//tmp.emplace_back(alpha, distance);//为了与opecv兼容，返回是先距离后角度
			tmp.emplace_back(distance, alpha);
			distance += rho;
		}
		alpha += theta;
	}

	//将结果填充到用户空间
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
//brief:概率霍夫直线检测
//becare:已经根据 /opencv3.4.1/opencv/sources/modules/imgproc/src/hough.cpp订正过
void HoughLinesP(cv::InputArray src, cv::OutputArray lines, double rho, double theta, int threshold,
	double min_line_length, double max_gap_length){
	cv::Mat in = src.getMat();
	assert(in.type() == CV_8UC1);
	assert(in.isContinuous());

	//获取候选点，并初始化掩码（状态）矩阵
	cv::Mat _state = cv::Mat::zeros(in.size(), CV_8UC1);
	uint8_t* state = _state.data;
	size_t step = _state.step;

	cv::Size size = in.size();
	std::vector<std::pair<int, int>>candidate;
	auto get_candi = [&](int x, int y, uint8_t* cursor) {
		if (*cursor != UINT8_MAX)
			return;
		state[y*step + x] = UINT8_MAX;//并把对应位置的_state标记可用
		candidate.emplace_back(x,y);
	};
	detail::geometricTriversal(in, get_candi);

	//初始化累加器
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
	//随机添加新的点
	while (unvisited_end > 0) {
		int pos = rand()%unvisited_end;
		int x = candidate[pos].first;
		int y = candidate[pos].second;
		std::swap(candidate[pos], candidate[--unvisited_end]);//采用延迟删除的方式

		if (!state[y * step + x])//已经处理过，隶属于其他直线
			continue;

		//累加计数器，并把已经符合条件的点删除（延迟）
		//FIXME:每一次只取值最大的进行处理，之前理解错了！！！
		int max_vote = 0;
		//double max_rho;//FIXME:而且这种实现并不需要记录距离！！
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

		//沿着两个方向，寻找间断距离小于max_gap_distance的线段
		double cos_a = std::cos(max_angle);
		double sin_a = std::sin(max_angle);
		double dx0, dy0;
		if (fabs(cos_a) < fabs(sin_a)) {
			//x方向增长的快
			dx0 = sin_a < 0 ? -1 : 1;
			dy0 = fabs(sin_a) < digital::k_EPS ? 0 : dx0 * cos_a / sin_a;
		}
		else {
			//y方向增长的快
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
		//判断线段长度是否满足条件
		//bool good_line = GET_DISTANCE2D(line_end[0], line_end[1]) >= min_line_length;//opencv中采用了直接判断delta_x delta_y的方式
		bool good_line = (abs(line_end[0].x - line_end[1].x) >= min_line_length ||
			abs(line_end[0].y - line_end[1].y) >= min_line_length);
		dx0 = -dx0, dy0 = -dy0;
		//第二次沿着直线方向遍历，目的在于清除这些点的可用性
		//注意这里的思想，无论好坏与否，这些点都不在参与计算，
		//这样必然存在误判，但也正是本算法快的原因之一
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
					*mask = 0;//使得其不可用
					if (!good_line)
						continue;

					//如果这些点作为线段保存出来了，那么他们对霍夫
					//空间的贡献必须全部消除
					//注意这里存在误判，即把尚未贡献的人多删了一份
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
				//此时的退出条件是：把该线段更新完即可
				if (i == line_end[k].x && j == line_end[k].y)
					break;
			}
		}

		if (good_line) {
			//需要记录到结果空间中
			vec.emplace_back(line_end[0].x, line_end[0].y,
				line_end[1].x, line_end[1].y);
			//for test
			//__test_mask(_state);
		}
	}

	//将结果赋予用户空间
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


//brief:将探测的直线叠加到图像上
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

	//与二值图像做一个交操作
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
			if (*ptr == 0) {//其中非交集部分取原图像的值，从而使得直线受限于二值图像
				for (int c = 0; c < channels; ++c) 
					it[c] = cur[c];
			}
			it += elemsize;
			cur += elemsize;
			++ptr;
		}
	}
}

//brief；
void matchTemplate(cv::InputArray src, cv::InputArray templ, cv::OutputArray dst, int method) {
	assert(src.depth() == CV_8U || src.depth() == CV_32F);
	assert(templ.depth() == CV_8U || templ.depth() == CV_32F);

	cv::Mat in = src.getMat();
	cv::Mat kernel = templ.getMat();
	assert(kernel.cols <= in.cols && kernel.rows <= in.rows);

	//becare:这里是少见的采用了VALID卷积的地方
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

//brief:该方法的本质就是以L2距离作为评判两幅图像相似性的依据，因此值越小越匹配
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
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L2欧式空间距离
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

//brief:在MatchSqdiff基础之上，进行了归一化，从而减少尺寸的敏感性
void MatchSqdiffNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;

	float square_sum = 0.;
	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//模板的归一化参数是恒定的

	auto squared_difference = [start, size, square_sum, jump](uint8_t** arr, uint8_t* cur) {
		double sum = 0.;
		float win_square_sum = 0.;
		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_square_sum += *iter * *iter;
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L2欧式空间距离
				sum += tmp * tmp;
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = static_cast<float>(sum * std::pow(win_square_sum*square_sum, -0.5));//归一化
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1), \
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:该方法的本质就是以两个函数的相关作为评价相似性的标准，因此值越大越匹配
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

//brief:在cross correlation的基础之上，进行归一化操作，减少尺寸敏感性
void MatchCcorrNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	
	float square_sum = 0.;
	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//模板的归一化参数是恒定的

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
		*data = static_cast<float>(sum * std::pow(win_square_sum*square_sum,-0.5));//归一化
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:该方法的本质是将两个图像的相关系数作为评价标准，值越大越匹配
//becare:实际上在没有进行归一化的时候，实际采用了协方差作为其评价准测，归一化后为相关系数
//       此时值越接近1越匹配，越接近-1越不匹配
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
	detail::grayscaleTransform(templ, get_mean);//模板的归一化参数是恒定的
	double templ_mean = templ_sum * total_1;

	//brief:在一遍遍历中完成均值和协方差的统计
	auto squared_difference = [start, size, templ_sum, templ_mean, total_1, total, jump]\
		(uint8_t** arr, uint8_t* cur) {
		double sum = 0.;
		double win_sum = 0.;
		double win_mean = 0.;
		auto ptr = start;

		//将公式展开后，可以同时统计均值和协方差
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

//brief:该方法的本质是将两个图像的相关系数作为评价标准，值越大越匹配
//becare:实际上在没有进行归一化的时候，实际采用了协方差作为其评价准测，归一化后为相关系数
//       此时值越接近1越匹配，越接近-1越不匹配
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
	detail::grayscaleTransform(templ, get_mean_sigma);//模板的归一化参数是恒定的
	templ_mean = templ_sum * total_1;
	templ_sigma = templ_sigma + templ_mean * templ_mean * total - 2 * templ_mean * templ_sum;

	//brief:在一遍遍历中完成均值和协方差的统计
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

//brief:该方法的本质就是以L1范数作为评判两幅图像相似性的依据，因此值越小越匹配
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
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L1欧式空间距离
				sum += tmp;//和L2范数的唯一区别
			}
			ptr += jump;
		}
		float* data = reinterpret_cast<float*>(cur);
		*data = sum;
	};
	detail::filter2DNonLinear(src, dst, templ, CV_32F, squared_difference, cv::Point(-1,-1),\
		cv::BORDER_DEFAULT, detail::VALID);
}

//brief:该方法的本质就是以L1范数作为评判两幅图像相似性的依据，因此值越小越匹配
void MatchAbsdiffNormed::operator()(cv::Mat& src, cv::Mat& templ, cv::Mat& dst) {
	auto start = templ.data;
	cv::Size size = templ.size();
	size_t jump = templ.step;
	double square_sum = 0.;

	auto get_square_sum = [&square_sum](uint8_t* cursor) {
		square_sum += *cursor * *cursor;
	};
	detail::grayscaleTransform(templ, get_square_sum);//模板的归一化参数是恒定的

	auto squared_difference = [start, size, square_sum, jump](uint8_t** arr, uint8_t* cur) {
		float sum = 0;
		double win_square = 0.;

		auto ptr = start;
		for (int y = 0; y < size.height; ++y) {
			auto iter = arr[y];
			for (int x = 0; x < size.width; ++x) {
				win_square += *iter * *iter;
				float tmp = static_cast<float>(fabs(*iter++ - ptr[x]));//L1欧式空间距离
				sum += tmp;//和L2范数的唯一区别
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