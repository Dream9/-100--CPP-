#include"Solution/morphology_transform.h"
#include"Solution/base.h"
#include"Solution/type_extension.h"

#include<unordered_map>

namespace {

//brief:获得结构元的反射，操作类似于卷积核的翻转
inline void getReflex(cv::Mat& m) {
	return cv::flip(m,m,-1);
}

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

void MorphGradient::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	//TODO
}
void MorphTopHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	//TODO
}
void MorphBlackHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	//TODO
}
}//!namespace