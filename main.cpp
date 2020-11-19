#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include "des.h"

using namespace std ;


void StrFromBlock(char * str , const Block & block)
{
	memset(str , 0 , 8) ;//将8个字节全部置0
	for(size_t i = 0 ; i < block.size() ; ++i)
	{
		if(true == block[i])//第i位为1
			*((unsigned char *)(str) + i / 8) |= (1 << (7 - i % 8)) ;
	}
}

// 二进制010串
// 不够64位，8字节如何进行加密,构建数据块存在问题
// 如果str不够8字节，需要特殊处理
void BlockFromStr(Block & block , const char * str)
{
	for(size_t i = 0 ; i < block.size() ; ++i)
	{
		if(0 != (*((unsigned char *)(str) + i / 8) & (1 << (7 - i % 8))))
			block[i] = true ;
		else 	block[i] = false ;
	}	
}

int main(int argc , char * argv[])
{
	if(argc < 2 || argv[1][0] != '-'){
		// 参数个数非法
    cout<<"参数输入有误"<<endl;
    cout<<"正在退出"<<endl;
    exit(-1);
  }
	
	Method method ;//记录运算方式（加密/解密）
	switch(argv[1][1])
	{
		case 'e'://加密
			method = e ;
      cout<<"正在进行加密......"<<endl;
			break ;
		case 'd'://解密
			method = d ;
      cout<<"正在进行解密......"<<endl;
			break ;
		default:
			// 输入参数有误
      cout<<"参数输入有误"<<endl;
			break ;
	}

	ifstream srcFile(argv[2]) ;//输入文件
	ofstream tarFile(argv[3]) ;//输出文件
	if(!srcFile || !tarFile){
    cout<<"打开文件失败"<<endl;
    exit(-1);
  }
	//文件打开失败

	// 计算文件大小
    long file_size ,number_of_blocks;
    srcFile.seekg(0,srcFile.end);
    size_t size = srcFile.tellg();
    number_of_blocks=size/8 + ((size%8)?1:0);
	srcFile.seekg(0,srcFile.beg);
    cout << "File size is : " << size << endl;
    cout << " Number of blocks is : " << number_of_blocks << endl;

	Block block , bkey ;//数据块和密钥
	BlockFromStr(bkey , argv[4]) ;//获取密钥
	unsigned short int padding = 8 - size%8;
	char buffer[8] ;
	cout<<"需要填充的字符数 : "<< padding <<endl;

	// 改写读取文件的循环,结束的逻辑不是end_of_file,最后一次读取需要作出处理,并非每次才能读取8个字节
	// 以8字节分段加·解密数据
	while(1)
	{
		memset(buffer , 0 , 8) ;//将8个字节置0
		srcFile.read(buffer , 8) ;// 每次从源中读取8个字节数据
		/*
		size_t extracted = srcFile.gcount(); // 上一次读入的字符数
		cout<<"gcount:   "<< extracted<<endl; 
		*/
		BlockFromStr(block , buffer) ;//构建数据块
		des(block , bkey , method) ;
		StrFromBlock(buffer , block) ;//获取运算后的数据
		tarFile.write(buffer , 8) ;//写入目标文件
		// 进行最后一次读取并跳出循环,需
	}
	tarFile.close();
	srcFile.close();
	return 0;
}