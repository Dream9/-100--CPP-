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

//brief:��ýṹԪ�ķ��䣬���������ھ���˵ķ�ת
inline void getReflex(cv::Mat& m) {
	return cv::flip(m, m, -1);
}

//brief:dfs�����ͨ����
int __dfs_connect4(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth);
int __dfs_connect8(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth);

//brief:quick_union�㷨�Ļ���
int __find(std::map<int, int>&dict, int val);

//brief:����������
//becare:���ڼ������4�ڽ�/8�ڽӵĹ�ʽ������ȡ��֮�ⶼһ�������û����������ֻҪ�Ѿ�ȡ����
//       ��ô�õ��ľ��ǻ���8�ڽӵ�������
int __connect_number(int* vec);

//brief:hildretch
void __hilditch(cv::Mat& src, cv::Mat& dst);

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

//brief:����ͬ����ͨ������ע��Ψһֵ
//becare:Ŀǰֻ����CV_8UC1����չ
int getConnectComponent(cv::InputArray src, cv::OutputArray dst, int flag, bool use_dfs) {
	assert(src.type() == CV_8UC1);

	cv::Mat in = src.getMat();
	cv::Size size = in.size();
	dst.create(size, src.type());
	cv::Mat out = dst.getMat();
	memset(out.data, 0, out.total()*out.elemSize());

	uint8_t* cur = in.data;
	if (use_dfs) {
		//becare:dfs��ͼ�����������ʱ����ջ����ķ���
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
		//����quick_union�㷨����dfs��һ�㣬����û��ջ����ķ��� 
		std::map<int, int> dict;
		int counter = 1;
		auto iter = out.data;

		//FIXME:�����������ƵĴ����Ƴ�
		if (flag == LINE_4) {
			auto set_value4 = [size, &iter, &counter, &dict](int x, int y, uint8_t* cur) {
				if (*cur == 0) {
					++iter;
					return;
				}

				//������ͨ����ָʾ
				int top = 0, left = 0;
				left = x >= 1 ? iter[-1] : left;
				top = y >= 1 ? iter[-size.width] : top;

				int prev = std::max(top, left);
				if (prev == 0) {
					*iter++ = counter++;
				}
				else {
					//����union����
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

				//������ͨ����ָʾ
				int top = 0, left = 0, tl = 0;
				left = x >= 1 ? iter[-1] : left;
				top = y >= 1 ? iter[-size.width] : top;
				tl = y >= 1 && x >= 1 ? iter[-size.width - 1] : tl;

				int prev = std::max(top, std::max(tl, left));
				if (prev == 0) {
					*iter++ = counter++;
				}
				else {
					//����union����
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

		//���������еڶ����������ͬһ����ͨ������ָ��ͬһ�����ձ�ʶ����Ҳ�Ǳ�dfs����ԭ��
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

//brief:���ÿ�����ص��ڽ���
//becare:��ͬ������������õ������
void setConnectNumber(cv::InputArray src, cv::OutputArray dst, int flag) {
	assert(src.type() == CV_8UC1);

	cv::Mat in = src.getMat();
	cv::Size size = in.size();

	dst.create(size, in.type());
	cv::Mat out = dst.getMat();
	memset(out.data, 0, out.total()*out.elemSize());

	//ȫ��8�����򣬸���flag����4/8�ڽ�
	//int orientation[][2] = { {0,-1},{0,1},{-1,0},{1,0},
	//						{-1,-1},{-1,1},{1,-1},{1,1} };

	//����ʱ�뷽����֯
	int orientation[][2] = { {1,0},{1,-1},{0,-1},{-1,-1},
							{-1,0},{-1,1},{0,1},{1,1} };
	int end_index = sizeof orientation / sizeof (int*);

	if (flag != LINE_4 && flag != LINE_8) {
		dealException(digital::kParameterNotMatch);
		return ;
	}
	uint8_t value = flag == LINE_8;//LINE_8����������Ҫȡ��

	auto iter = out.data;
	auto calc_connection = [&iter, end_index, value, orientation, size](int x, int y, uint8_t* cursor) {
		if (*cursor == 0) {
			++iter;
			return;
		}

		//ͳ���ڽ���
		int vec[8];//��ǰ��¼����
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
		//���ĸ��������������
		*iter++ = static_cast<uint8_t>(__connect_number(vec));
	};
	detail::geometricTriversal(in, calc_connection);
}

//brief:ϸ���㷨
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
			int vec[8];//��ǰ��¼����
			for (int i = 0; i < end_index; ++i) {
				int a = x + orientation[i][0];
				int b = y + orientation[i][1];
				if (OUT_RANGE(a, b, size.width, size.height)) {
					vec[i] = 0;//�˴�����4�ڽ�
					continue;
				}
				vec[i] = cursor[orientation[i][1] * size.width + orientation[i][0]] != 0 ?
					1 - 0 : 0;
			}

			//����1��4����������
			//uint8_t connection_4 = iter[1] != 0 + iter[-1] != 0 +
			//	iter[-size.width] != 0 + iter[size.width] != 0;
			uint8_t connection_4 = vec[0] + vec[2] + vec[4] + vec[6];
			//++iter;
			if (connection_4 == 4)
				return;

			//����2��4������Ϊ1�����ڿ�ɾ�����˵㣩
			if (__connect_number(vec) != 1)
				return;

			//����3��8����������������ǰ�����أ�˵���������4����ʱ��ë�̣�
			if (3 > std::accumulate(vec, vec + 8, 0))
				return;

			//��ô������ǿ�ɾ����
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

//brief:��̬ѧ�ݶȣ��������͵Ľ��-��ʴ�Ľ��
void MorphGradient::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	detail::dilate(src, tmp, kernel, anchor, iterations);
	detail::erode(src, dst, kernel, anchor, iterations);
	dst = tmp - dst;
}

//brief:�׶�ñ�任����Ҫ����һ���ܴ�ĽṹԪ�����п�������ȥ���󲿷ֵİ�ɫǰ�����壬����������Ӱģʽ��Ȼ�������
//      ��Ӱģʽ���Ӷ�ʹ�ù��վ��ȣ��ڵ�ñ�任�ʺ��ں�ɫǰ�����壬��ɫ�������գ������෴
void MorphTopHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	MorphDict[detail::MORPH_OPEN]->operator()(src, tmp, kernel, anchor, iterations);
	dst = src - tmp;
}

//brief:�ڵ�ñ�任������μ��׶�ñ
void MorphBlackHat::operator()(cv::Mat& src, cv::Mat& dst, cv::Mat& kernel, cv::Point anchor , int iterations) {
	cv::Mat tmp;
	MorphDict[detail::MORPH_CLOSE]->operator()(src, tmp, kernel, anchor, iterations);
	dst = tmp - src;

}

//brief:dfs����
int __dfs_connect4(cv::Mat& src, cv::Mat& dst, int x, int y, uint8_t value, int depth) {
	if (depth > kSTACK_THRESHOLD) {
		coutInfo(" warning, it may stack overflow using dfs");
		return -1;
	}
	if (OUT_RANGE(x, y, src.cols, src.rows) || dst.at<uint8_t>(y, x)>0 || src.at<uint8_t>(y, x) != UINT8_MAX)
		return 0;

	//dfs��ȥ����
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

	//dfs��ȥ����
	dst.at<uint8_t>(y, x) = value;
	int orientation[][2] = { {-1, 0},{0, -1},{1, 0},{0, 1},
								{-1, -1},{1, -1},{-1, 1},{1, 1} };
	for (auto cur : orientation)
		__dfs_connect8(src, dst, x + cur[0], y + cur[1], value,depth+1);

	return 0;
}

//brief:˼·�μ����㷨��
//becare:�ݹ����ʱ�Ὣ��ǰָ�򶼵�����head��������quick��ԭ��
int __find(std::map<int, int>&dict, int val) {
	auto iter = dict.find(val);
	if (iter == dict.end()) {
		dict.insert({ val,val });
		return val;
	}

	int head = iter->second;
	if (head == val)
		return val;
	//becare������������·��ѹ������û����Ȩ
	head = __find(dict, head);
	iter->second = head;

	return head;
}

#define NEIGHBOR_NUMBER 8
//brief:����������
//     �����vec�����Ǵ�����һ��4����㿪ʼ����˳ʱ���¼��
//     ��˱���������8��Ԫ�أ�����1��ʾǰ����0��ʾ����
int __connect_number(int* vec) {
	assert(vec);

	int counter = 0;
	for (int i = 0; i < 8; i += 2) {//0��2��4��6λ��Ϊ4��������
		//FIXME:���һ��ֵ���������
		//counter += vec[i] - vec[i] * vec[i + 1] * vec[(i + 2];
		counter += vec[i] - vec[i] * vec[i + 1] * vec[(i + 2) % NEIGHBOR_NUMBER];
	}
	return counter;
}
#undef NEIGHBOR_NUMBER

#define CONNECT8_TRUE_VALUE 0
//brief:����Hilditch�㷨��ϸ��
void __hilditch(cv::Mat& src, cv::Mat& dst) {
	auto iter = dst.data;
	auto set_one = [&iter](uint8_t* cursor) {
		*iter++ = *cursor == 0 ? 0 : UINT8_MAX;
	};
	detail::grayscaleTransform(src, set_one);//���01��¼�Ķ�ֵͼ��

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
		//���ֵ���ǰ����������Ϣ
		for (int i = 0; i < end_index; ++i) {
			int a = x + orientation[i][0];
			int b = y + orientation[i][1];
			if (OUT_RANGE(a, b, size.width, size.height)) {
				vec[i] = 1 - CONNECT8_TRUE_VALUE;//�˴�����8�ڽ�
				continue;
			}
			vec[i] = iter[orientation[i][1] * size.width + orientation[i][0]] != 0 ?
				CONNECT8_TRUE_VALUE : 1 - CONNECT8_TRUE_VALUE;
		}

		//���ֵ������˴�ʱ���ѷ���������Ϣ
		bool previous_mark[4];
		for (int i = 0; i < sizeof previous_mark / sizeof(bool); ++i) {
			previous_mark[i] = (vec[i + 1] == CONNECT8_TRUE_VALUE) &&
				(cursor[orientation[i + 1][1] * size.width + orientation[i + 1][0]] == kMarked);
		}
		++iter;

		//����1��4���������ģ���ֹ�ն���
		if (vec[0] + vec[2] + vec[4] +vec[6] == 0)//!!!becare:�����vecȡ���ˣ�����
			return;

		//����2���ڽ��б��ֵ���֮ǰ8������Ϊ1�����ѱ�����أ�
		if (__connect_number(vec) != 1)
			return;

		//����3���ڽ��б��ֵ���֮ǰ8����������������ǰ�����أ���ֹ�߶α�������С��, ��������6��ǰ�����أ���ֹ���Ȱ��ݣ�����ֹ�ֲ棩
		int background = std::accumulate(vec, vec + 8, 0);//becare:vec�Ǽ�¼�Ļ��ڰ����ӵ�0-1ֵ����ȡ���ġ�����
		if(background > 6 || background < 2)
			return;

		//����4��ȥ�������ѱ�����غ�8���������ٻ���1��ֵ
		if (background + std::accumulate(previous_mark, previous_mark + 4, 0) > 7)
			return;

		//����5:���ɾ�������ѱ�����غ󣬸õ����¼������������Ȼ��������
		for (int i = 0; i < sizeof previous_mark / sizeof(bool); ++i) {
			if (!previous_mark[i])
				continue;
			vec[i + 1] = 1;
			if (__connect_number(vec) != 1)
				return;
			vec[i + 1] = 0;
		}

		//��ô�����Ӧ�������
		*cursor = kMarked;
		end_iteration = false;
	};
	
	auto set_zero = [](uint8_t* cursor) {
		*cursor = *cursor == UINT8_MAX ? UINT8_MAX : 0;
	};

	do{//������ֱ������
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