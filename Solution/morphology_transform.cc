#include"Solution/morphology_transform.h"
#include"Solution/base.h"
#include"Solution/type_extension.h"

#include<unordered_map>

namespace {

//brief:��ýṹԪ�ķ��䣬���������ھ���˵ķ�ת
inline void getReflex(cv::Mat& m) {
	return cv::flip(m,m,-1);
}

//���ڹ��������ͷ������
class MorphFactory;
std::unordered_map<int, MorphFactory*> MorphDict;

class MorphFactory {
public:
	virtual void operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) = 0;

	//����
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

//brief:ԭ��Ϊ�Ҷȼ���̬ѧ���ͣ���ÿ�ε�������SE����Ԫ���ص������ڵ����ֵ
//becare:Ŀǰֻ����CV_8U����չ����࿼����4ͨ��
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

		//�����ṹԪ�����ص�����
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

		//����ṹ���ڵ����ֵ���Ӷ�ʵ������
		for (int c = 0; c < channels; ++c)
			cur[c] = val[c];
	};

	cv::Mat tmp = src.clone();
	while (iterations-- > 0) {
		detail::filter2DNonLinear(tmp, dst, kernel, src.depth(), make_dilate, anchor);
		::memcpy(tmp.data, dst.data, tmp.total()*tmp.elemSize());
	}
}

//brief:ԭ��Ϊ�Ҷȼ���̬ѧ��ʴ����ÿ�ε�������SEԪ���ص������ڵ���Сֵ
//becare:Ŀǰֻ����CV_8U����չ����࿼����4ͨ��
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

		//�����ṹԪ�����ص�����
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

		//����ṹ���ڵ����ֵ���Ӷ�ʵ������
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


//brief:�߼���̬ѧ�任
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

//brief:������������ʴ�����ͣ�����ȥ��С�ڽṹԪ�������㣬����Ҷȱ�ͣ��ǰ׶�ñ�任�Ļ���
void MorphOpen::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	detail::erode(src,tmp,kernel,anchor,iterations);
	detail::dilate(tmp,dst,kernel,anchor,iterations);
}

//brief:�ղ����������ͺ���ʴ������ȥ��С�ڽṹԪ���ĺڵ㣬�Ǻڵ�ñ�任�Ļ���
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