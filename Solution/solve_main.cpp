#include"Solution/solution.h"

//#include"Solution/channel_swap.h"
#include"Solution/grayscale.h"

using namespace digital;
int main(){
	//ChannelSwap cs001("F:/yzhh/数字图像处理100问及解答/Question_01_10/imori.jpg",
	//	              true);
	//Solve(cs001);

	Grayscale gs002("F:/yzhh/数字图像处理100问及解答/Question_01_10/imori.jpg",
		              true);
	Solve(gs002);
}