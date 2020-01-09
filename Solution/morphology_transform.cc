#include"Solution/morphology_transform.h"
#include"Solution/base.h"
#include"Solution/geometric_transform.h"
#include"Solution/grayscale_transfrom.h"
#include"Solution/type_extension.h"



#include"Solution/solution.h"
#include<opencv2/highgui.hpp>



#include<unordered_map>
#include<map>
#include<numeric>

#define OUT_RANGE(x,y,width,height) (x<0 || y<0 || x>=width || y>=height)

namespace {

const int kSTACK_THRESHOLD = 2000;

//brief:获得结构元的反射，操作类似于卷积核的翻转
inline void getReflex(cv::Mat& m) {
	return cv::flip(m, m, -1);
}

//brief:dfs填充连通分量
int __dfs_connect4(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth);
int __dfs_connect8(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth);

//brief:quick_union算法的基础
int __find(std::map<int, int>&dict, int val);

//brief:计算连接数
//becare:由于计算基于4邻接/8邻接的公式，除了取反之外都一样，这用户传入的数组只要已经取反了
//       那么得到的就是基于8邻接的连接数
int __connect_number(int* vec);

//brief:hildretch
void __hilditch(cv::Mat& src, cv::Mat& dst);

//基于工厂的类型反射机制
class MorphFactory;
std::unordered_map<int, MorphFactory*> MorphDict;

class MorphFactory {
public:
	virtual void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) = 0;

	//创建
	static MorphFactory* create(int op) {
		auto iter = MorphDict.find(op);
		return iter != MorphDict.end() ? iter->second : nullptr;
	}
};

class MorphDilate:public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphErode :public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphOpen :public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphClose:public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphGradient :public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphTopHat :public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class MorphBlackHat :public MorphFactory {
public:
	void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) override;
};

class InitMorph : digital::noncopyable {
public:
	InitMorph() {
		MorphDict.insert({ detail::MORPH_DILATE,new MorphDilate });
		MorphDict.insert({ detail::MORPH_ERODE,new MorphErode});

		MorphDict.insert({ detail::MORPH_OPEN,new MorphOpen});
		MorphDict.insert({ detail::MORPH_CLOSE,new MorphClose });

		MorphDict.insert({ detail::MORPH_GRADIENT,new MorphGradient});

		MorphDict.insert({ detail::MORPH_TOPHAT,new MorphTopHat });
		MorphDict.insert({ detail::MORPH_BLACKHAT,new MorphBlackHat });
	}

	~InitMorph() {
		std::for_each(MorphDict.begin(), MorphDict.end(), [](std::pair<int,MorphFactory*> item) {
			delete item.second;
		});
	}
};

InitMorph __global_morphology_init_object;

}//!namespace 

namespace detail {

//brief:原理为灰度级形态学膨胀，即每次迭代赋予SE反射元件重叠部分内的最大值
//becare:目前只做了CV_8U的扩展，最多考虑了4通道
void dilate(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	assert(src.data != dst.data);
	assert(src.depth() == CV_8U);
	assert(src.channels() <= 4);
	assert(kernel.depth() == CV_8U);
	assert(kernel.isContinuous());

	dst = cv::Mat(src.size(), src.type());

	cv::Mat reflexK = kernel.clone();
	getReflex(kernel);
	auto begin = kernel.data;
	auto size = kernel.size();
	auto channels = src.channels();

	auto make_dilate = [=](uint8_t** arr, uint8_t* cur) {
		uint8_t val[4] = { 0 };
		auto left = begin;

		//遍历结构元件的重叠部分
		for (int i = 0; i < size.height; ++i) {
			for (int j = 0; j < size.width; ++j) {
				if (*left++ == 0) {
					continue;
				}
				uint8_t* data = arr[i] + j * channels;
				for (int c = 0; c < channels; ++c) 
					val[c] = std::max(val[c], data[c]);
			}
		}

		//赋予结构件内的最大值，从而实现膨胀
		for (int c = 0; c < channels; ++c)
			cur[c] = val[c];
	};

	cv::Mat tmp = src.clone();
	while (iterations-- > 0) {
		detail::filter2DNonLinear(tmp, dst, kernel, src.depth(), make_dilate, anchor);
		::memcpy(tmp.data, dst.data, tmp.total()*tmp.elemSize());
	}
}

//brief:原理为灰度级形态学侵蚀，即每次迭代赋予SE元件重叠部分内的最小值
//becare:目前只做了CV_8U的扩展，最多考虑了4通道
void erode(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	assert(src.data != dst.data);
	assert(src.depth() == CV_8U);
	assert(src.channels() <= 4);
	assert(kernel.depth() == CV_8U);
	assert(kernel.isContinuous());

	dst = cv::Mat(src.size(), src.type());

	auto begin = kernel.data;
	auto size = kernel.size();
	auto channels = src.channels();

	auto make_erode = [=](uint8_t** arr, uint8_t* cur) {
		uint8_t val[4] = { UINT8_MAX, UINT8_MAX, UINT8_MAX,UINT8_MAX };
		auto left = begin;

		//遍历结构元件的重叠部分
		for (int i = 0; i < size.height; ++i) {
			for (int j = 0; j < size.width; ++j) {
				if (*left++ == 0) {
					continue;
				}
				uint8_t* data = arr[i] + j * channels;
				for (int c = 0; c < channels; ++c) 
					val[c] = std::min(val[c], data[c]);
			}
		}

		//赋予结构件内的最大值，从而实现膨胀
		for (int c = 0; c < channels; ++c)
			cur[c] = val[c];
	};

	cv::Mat tmp = src.clone();
	while (iterations > 0) {
		--iterations;
		detail::filter2DNonLinear(tmp, dst, kernel, src.depth(), make_erode, anchor);
		if(iterations > 0)
			::memcpy(tmp.data, dst.data, tmp.total()*tmp.elemSize());
	}
}


//brief:高级形态学变换
void morphologyEx(cv::Mat& src, cv::Mat& dst, int op, cv::Mat& kernel, cv::Point anchor, int iteration) {

	auto morph_operator = MorphFactory::create(op);
	if (!morph_operator) {
		dealException(digital::kParameterNotMatch);
		return;
	}

	morph_operator->operator()(src, dst, kernel, anchor, iteration);
}

//brief:将相同的连通分量标注成唯一值
//becare:目前只做了CV_8UC1的扩展
int getConnectComponent(cv::InputArray src, cv::OutputArray dst, int flag, bool use_dfs) {
	assert(src.type() == CV_8UC1);

	cv::Mat in = src.getMat();
	cv::Size size = in.size();
	dst.create(size, src.type());
	cv::Mat out = dst.getMat();
	memset(out.data, 0, out.total()*out.elemSize());

	uint8_t* cur = in.data;
	if (use_dfs) {
		//becare:dfs在图像中物体过大时存在栈溢出的风险
		int(*dfs)(cv::Mat&, cv::Mat&, int, int, uint8_t, int);
		dfs = flag == LineTypes::LINE_4 ? __dfs_connect4 : __dfs_connect8;

		uint8_t counter = 1;
		for (int y = 0; y < size.height; ++y) {
			for (int x = 0; x < size.width; ++x) {
				if (*cur++ == 0)
					continue;
				dfs(in, out, x, y, counter++, 1);
			}
		}
		return counter - 1;
	}
	else {
		//采用quick_union算法，比dfs慢一点，但是没有栈溢出的风险 
		std::map<int, int> dict;
		int counter = 1;
		auto iter = out.data;

		//FIXME:把这两段相似的代码移出
		if (flag == LINE_4) {
			auto set_value4 = [size, &iter, &counter, &dict](int x, int y, uint8_t* cur) {
				if (*cur == 0) {
					++iter;
					return;
				}

				//更新连通分量指示
				int top = 0, left = 0;
				left = x >= 1 ? iter[-1] : left;
				top = y >= 1 ? iter[-size.width] : top;

				int prev = std::max(top, left);
				if (prev == 0) {
					*iter++ = counter++;
				}
				else {
					//进行union操作
					prev = __find(dict, prev);
					*iter++ = prev;
					if (top != 0 && top != prev)
						dict[top] = prev;
					if (left != 0 && left != prev)
						dict[left] = prev;
				}
			};
			detail::geometricTriversal(in, set_value4);
		}
		else if(flag == LINE_8) {
			auto set_value8 = [size, &iter, &counter, &dict](int x, int y, uint8_t* cur) {
				if (*cur == 0) {
					++iter;
					return;
				}

				//更新连通分量指示
				int top = 0, left = 0, tl = 0;
				left = x >= 1 ? iter[-1] : left;
				top = y >= 1 ? iter[-size.width] : top;
				tl = y >= 1 && x >= 1 ? iter[-size.width - 1] : tl;

				int prev = std::max(top, std::max(tl, left));
				if (prev == 0) {
					*iter++ = counter++;
				}
				else {
					//进行union操作
					prev = __find(dict, prev);
					*iter++ = prev;
					if (top != 0 && top != prev)
						dict[top] = prev;
					if (left != 0 && left != prev)
						dict[left] = prev;
					if (tl != 0 && tl != prev)
						dict[tl] = prev;
				}
			};

			detail::geometricTriversal(in, set_value8);
		}
		else {
			dealException(digital::kParameterNotMatch);
			return -1;
		}

		//这里必须进行第二遍遍历，将同一个连通分量都指向同一个最终标识，这也是比dfs慢的原因
		iter = out.data;
		auto set_same_id = [&dict](uint8_t* cur) {
			if (*cur == 0)
				return;
			*cur= __find(dict, *cur);
		};
		detail::grayscaleTransform(out, set_same_id);
		return counter - 1;
	}
}

//brief:标记每个像素的邻接数
//becare:不同的连接数代表该点的特征
void setConnectNumber(cv::InputArray src, cv::OutputArray dst, int flag) {
	assert(src.type() == CV_8UC1);

	cv::Mat in = src.getMat();
	cv::Size size = in.size();

	dst.create(size, in.type());
	cv::Mat out = dst.getMat();
	memset(out.data, 0, out.total()*out.elemSize());

	//全部8个方向，根据flag决定4/8邻接
	//int orientation[][2] = { {0,-1},{0,1},{-1,0},{1,0},
	//						{-1,-1},{-1,1},{1,-1},{1,1} };

	//按逆时针方向组织
	int orientation[][2] = { {1,0},{1,-1},{0,-1},{-1,-1},
							{-1,0},{-1,1},{0,1},{1,1} };
	int end_index = sizeof orientation / sizeof (int*);

	if (flag != LINE_4 && flag != LINE_8) {
		dealException(digital::kParameterNotMatch);
		return ;
	}
	uint8_t value = flag == LINE_8;//LINE_8的连接数需要取反

	auto iter = out.data;
	auto calc_connection = [&iter, end_index, value, orientation, size](int x, int y, uint8_t* cursor) {
		if (*cursor == 0) {
			++iter;
			return;
		}

		//统计邻接数
		int vec[8];//提前记录下来
		for (int i = 0; i < end_index; ++i) {
			int a = x + orientation[i][0];
			int b = y + orientation[i][1];
			if (OUT_RANGE(a, b, size.width, size.height)) {
				vec[i] = value;
				continue;
			}

			vec[i] = cursor[orientation[i][1] * size.width + orientation[i][0]] != 0 ?
				1 - value : value;
		}
		//从四个方向计算连接数
		*iter++ = static_cast<uint8_t>(__connect_number(vec));
	};
	detail::geometricTriversal(in, calc_connection);
}

//brief:细化算法
void thin(cv::InputArray src, cv::OutputArray dst, int flag) {
	assert(src.type() == CV_8UC1);
	cv::Mat in = src.getMat();

	cv::Size size = src.size();
	dst.create(size, src.type());
	cv::Mat out = dst.getMat();

	__hilditch(in, out);
	
	return;
		

	//
	auto iter = out.data;
	auto set_one = [&](uint8_t* cursor) {
		*iter++ = *cursor == 0 ? 0 : UINT8_MAX;
	};
	detail::grayscaleTransform(in, set_one);

	size_t jump = out.step;
	bool end_iteration = true;

	do{
		end_iteration = true;
		//cv::Mat tmp = out.clone();
		//iter = tmp.data;

		auto skeleton = [/*&iter,*/ &end_iteration, size, jump](int x, int y, uint8_t* cursor) {
			if (*cursor == 0) {
				//++iter;
				return;
			}
			int orientation[][2] = { {1,0},{1,-1},{0,-1},{-1,-1},
							{-1,0},{-1,1},{0,1},{1,1} };

			int end_index = sizeof orientation / sizeof(int*);
			int vec[8];//提前记录下来
			for (int i = 0; i < end_index; ++i) {
				int a = x + orientation[i][0];
				int b = y + orientation[i][1];
				if (OUT_RANGE(a, b, size.width, size.height)) {
					vec[i] = 0;//此处基于4邻接
					continue;
				}
				vec[i] = cursor[orientation[i][1] * size.width + orientation[i][0]] != 0 ?
					1 - 0 : 0;
			}

			//条件1：4邻域不是满的
			//uint8_t connection_4 = iter[1] != 0 + iter[-1] != 0 +
			//	iter[-size.width] != 0 + iter[size.width] != 0;
			uint8_t connection_4 = vec[0] + vec[2] + vec[4] + vec[6];
			//++iter;
			if (connection_4 == 4)
				return;

			//条件2：4连接数为1（属于可删除点或端点）
			if (__connect_number(vec) != 1)
				return;

			//条件3：8邻域内至少有三个前景像素（说明这个点是4连接时的毛刺）
			if (3 > std::accumulate(vec, vec + 8, 0))
				return;

			//那么这个点是可删除的
			*cursor = 0;
			end_iteration = false;
		};

		detail::geometricTriversal(out, skeleton);
		//assert(iter == tmp.data + tmp.total());

	} while (!end_iteration);

}


}//!namespace detail

namespace {

void MorphErode::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	detail::erode(src,dst,kernel,anchor,iterations);
}

void MorphDilate::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	detail::dilate(src,dst,kernel,anchor,iterations);
}

//brief:开操作，先侵蚀后膨胀，可以去除小于结构元件的亮点，整体灰度变低，是白顶帽变换的基础
void MorphOpen::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	detail::erode(src,tmp,kernel,anchor,iterations);
	detail::dilate(tmp,dst,kernel,anchor,iterations);
}

//brief:闭操作，先膨胀后侵蚀，可以去除小于结构元件的黑点，是黑底帽变换的基础
void MorphClose::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	detail::dilate(src,tmp,kernel,anchor,iterations);
	detail::erode(tmp,dst,kernel,anchor,iterations);
}

//brief:形态学梯度，即用膨胀的结果-腐蚀的结果
void MorphGradient::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	detail::dilate(src, tmp, kernel, anchor, iterations);
	detail::erode(src, dst, kernel, anchor, iterations);
	dst = tmp - dst;
}

//brief:白顶帽变换，主要是用一个很大的结构元件进行开操作，去除大部分的白色前景物体，保留背景阴影模式，然后剪掉该
//      阴影模式，从而使得光照均匀，黑底帽变换适合于黑色前景物体，白色背景光照，正好相反
void MorphTopHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	MorphDict[detail::MORPH_OPEN]->operator()(src, tmp, kernel, anchor, iterations);
	dst = src - tmp;
}

//brief:黑底帽变换，详情参见白顶帽
void MorphBlackHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	MorphDict[detail::MORPH_CLOSE]->operator()(src, tmp, kernel, anchor, iterations);
	dst = tmp - src;

}

//brief:dfs搜索
int __dfs_connect4(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth) {
	if (depth > kSTACK_THRESHOLD) {
		coutInfo(" warning, it may stack overflow using dfs");
		return -1;
	}
	if (OUT_RANGE(x, y, src.cols, src.rows) || dst.at<uint8_t>(y, x)>0 || src.at<uint8_t>(y, x) != UINT8_MAX)
		return 0;

	//dfs下去即可
	dst.at<uint8_t>(y, x) = value;
	int orientation[][2] = { {-1, 0},{0, -1},{1, 0},{0, 1} };
	for (auto cur : orientation)
		__dfs_connect4(src, dst, x + cur[0], y + cur[1], value,depth+1);

	return 0;
}
int __dfs_connect8(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth) {
	if (depth > kSTACK_THRESHOLD) {
		coutInfo(" warning, it may stack overflow using dfs");
		return -1;
	}
	if (OUT_RANGE(x, y, src.cols, src.rows) || dst.at<uint8_t>(y,x)>0 || src.at<uint8_t>(y, x) == 0)
		return 0;

	//dfs下去即可
	dst.at<uint8_t>(y, x) = value;
	int orientation[][2] = { {-1, 0},{0, -1},{1, 0},{0, 1},
								{-1, -1},{1, -1},{-1, 1},{1, 1} };
	for (auto cur : orientation)
		__dfs_connect8(src, dst, x + cur[0], y + cur[1], value,depth+1);

	return 0;
}

//brief:思路参见《算法》
//becare:递归回来时会将当前指向都调整到head，这是其quick的原因
int __find(std::map<int, int>&dict, int val) {
	auto iter = dict.find(val);
	if (iter == dict.end()) {
		dict.insert({ val,val });
		return val;
	}

	int head = iter->second;
	if (head == val)
		return val;
	//becare：这里这做了路径压缩，但没做加权
	head = __find(dict, head);
	iter->second = head;

	return head;
}

#define NEIGHBOR_NUMBER 8
//brief:计算连接数
//     传入的vec必须是从任意一个4邻域点开始沿着顺时针记录，
//     因此必须至少有8个元素，且用1表示前景，0表示背景
int __connect_number(int* vec) {
	assert(vec);

	int counter = 0;
	for (int i = 0; i < 8; i += 2) {//0，2，4，6位置为4邻域像素
		//FIXME:最后一个值有溢出风险
		//counter += vec[i] - vec[i] * vec[i + 1] * vec[(i + 2];
		counter += vec[i] - vec[i] * vec[i + 1] * vec[(i + 2) % NEIGHBOR_NUMBER];
	}
	return counter;
}
#undef NEIGHBOR_NUMBER

#define CONNECT8_TRUE_VALUE 0
//brief:基于Hilditch算法的细化
void __hilditch(cv::Mat& src, cv::Mat& dst) {
	auto iter = dst.data;
	auto set_one = [&iter](uint8_t* cursor) {
		*iter++ = *cursor == 0 ? 0 : UINT8_MAX;
	};
	detail::grayscaleTransform(src, set_one);//获得01记录的二值图像

	size_t jump = dst.step;
	bool end_iteration = true;
	cv::Size size = dst.size();
	const uint8_t kMarked = 128;

	auto skeleton = [&iter, &end_iteration, size, jump, kMarked](int x, int y, uint8_t* cursor) {
		if (*cursor == 0) {
			++iter;
			return;
		}
		int orientation[][2] = { {1,0},{1,-1},{0,-1},{-1,-1},
						{-1,0},{-1,1},{0,1},{1,1} };

		int end_index = sizeof orientation / sizeof(int*);
		int vec[8];
		//本轮迭代前邻域像素信息
		for (int i = 0; i < end_index; ++i) {
			int a = x + orientation[i][0];
			int b = y + orientation[i][1];
			if (OUT_RANGE(a, b, size.width, size.height)) {
				vec[i] = 1 - CONNECT8_TRUE_VALUE;//此处基于8邻接
				continue;
			}
			vec[i] = iter[orientation[i][1] * size.width + orientation[i][0]] != 0 ?
				CONNECT8_TRUE_VALUE : 1 - CONNECT8_TRUE_VALUE;
		}

		//本轮迭代到此处时的已访问像素信息
		bool previous_mark[4];
		for (int i = 0; i < sizeof previous_mark / sizeof(bool); ++i) {
			previous_mark[i] = (vec[i + 1] == CONNECT8_TRUE_VALUE) &&
				(cursor[orientation[i + 1][1] * size.width + orientation[i + 1][0]] == kMarked);
		}
		++iter;

		//条件1：4邻域不是满的（防止空洞）
		if (vec[0] + vec[2] + vec[4] +vec[6] == 0)//!!!becare:这里的vec取反了！！！
			return;

		//条件2：在进行本轮迭代之前8连接数为1（含已标记像素）
		if (__connect_number(vec) != 1)
			return;

		//条件3：在进行本轮迭代之前8邻域内至少有两个前景像素（防止线段被过度缩小）, 并且至多6个前景像素（防止过度凹陷，即防止分叉）
		int background = std::accumulate(vec, vec + 8, 0);//becare:vec是记录的基于八连接的0-1值，是取反的。。。
		if(background > 6 || background < 2)
			return;

		//条件4：去除本轮已标记像素后，8邻域内至少还有1个值
		if (background + std::accumulate(previous_mark, previous_mark + 4, 0) > 7)
			return;

		//条件5:逐个删除本轮已标记像素后，该点重新计算的连接数依然满足条件
		for (int i = 0; i < sizeof previous_mark / sizeof(bool); ++i) {
			if (!previous_mark[i])
				continue;
			vec[i + 1] = 1;
			if (__connect_number(vec) != 1)
				return;
			vec[i + 1] = 0;
		}

		//那么这个点应当被标记
		*cursor = kMarked;
		end_iteration = false;
	};
	
	auto set_zero = [](uint8_t* cursor) {
		*cursor = *cursor == UINT8_MAX ? UINT8_MAX : 0;
	};

	do{//迭代，直到收敛
		end_iteration = true;
		cv::Mat tmp = dst.clone();
		iter = tmp.data;

		detail::geometricTriversal(dst, skeleton);
		assert(iter == tmp.data + tmp.total());

		detail::grayscaleTransform(dst, set_zero);
#ifdef SHOW_PROCESS
		cv::namedWindow("processing",cv::WINDOW_NORMAL);
		cv::imshow("processing",dst);
		cv::waitKey(0);
		cv::destroyAllWindows();
#endif
	} while (!end_iteration);
}

#undef CONNECT8_TRUE_VALUE

}//!namespace

#undef OUT_RANGE