#include"Solution/solution.h"

//#include"Solution/channel_swap.h"
//#include"Solution/grayscale.h"
//#include"Solution/binarization.h"
//#include"Solution/otsu.h"
#include"Solution/reverse_hue.h"
#include"Solution/reduce_color.h"

using namespace digital;
int main(){
	//ChannelSwap cs001("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
	//	              true);
	//Solve(cs001);

	//Grayscale gs002("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
	//	              true);
	//Solve(gs002);

	//Binarization bz003("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
	//				  true);
	//Solve(bz003);

	//Otsu otsu004("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
	//				  true);
	//Solve(otsu004);
	
	//ReverseHue rh005("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
	//				  true);
	//Solve(rh005);

	ReduceColor rc006("F:/yzhh/����ͼ����100�ʼ����/Question_01_10/imori.jpg",
					  true);
	Solve(rc006);


}