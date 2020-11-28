#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include "des.h"

using namespace std ;

static FILE *input_file,*output_file;

void StrFromBlock(char * str , const Block & block)
{
	memset(str , 0 , 8) ;//将8个字节全部置0
	for(size_t i = 0 ; i < block.size() ; ++i)
	{
		if(true == block[i])//第i位为1
			*((unsigned char *)(str) + i / 8) |= (1 << (7 - i % 8)) ;
	}
}

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

  // 打开输入文件
  input_file = fopen(argv[2],"rb");
  if(!input_file){
    cout<<"无法打开文件"<<endl;
    return 1;
  }

  // 打开输出文件
  output_file = fopen(argv[3],"wb");
  if(!output_file){
    cout<<"无法打开输出文件"<<endl;
    return 1;
  }

  long file_size,number_of_blocks,block_count=0;

  fseek(input_file,0L,SEEK_END);//跳到文件尾
  file_size=ftell(input_file);
  cout<<"file_size: "<<file_size<<endl;
  fseek(input_file,0L,SEEK_SET);// 回到文件头

  number_of_blocks=file_size/8+((file_size%8)?1:0);
  cout<< "number_of_blocks: " <<number_of_blocks<<endl;

  Block block,bkey; // 数据块和密钥
  BlockFromStr(bkey,argv[4]);// 获取密钥
  char text[8];
  char buffer[8]; // 分组数据
  unsigned short int padding = 8 -file_size%8; // 缺失字节
  cout<<"padding: "<<padding<<endl;

  while(fread(buffer,1,8,input_file)){ // 一直读取直到EOF==0
    block_count++;//从第一个数据块开始处理
    memset(text,0,8);
    if(block_count==number_of_blocks){
    cout<<"正在处理最后一个数据块"<<endl;
      if(method==e){
        if(padding<8){ // 需要构造辅助序列
          memset((buffer+8-padding),(unsigned char)padding,padding);
        }
        BlockFromStr(block,buffer);
        des(block,bkey,method);
        StrFromBlock(text,block);
        fwrite(text,1,8,output_file);

        if(padding==8){
          memset(buffer,(unsigned char)padding,8);
          des(block,bkey,method);
          StrFromBlock(text,block);
          fwrite(buffer,1,8,output_file);
        }
      }else{
        BlockFromStr(block,buffer);
        des(block,bkey,method);
        StrFromBlock(text,block);
        padding=text[7];
        cout<<"解密的padding(buffer[7]) "<<text[7]<<endl;
        cout<<"解密的padding "<<padding<<endl;

        if(padding<8)
          fwrite(text,1,8-padding,output_file);
      }
    }else{
      BlockFromStr(block,buffer);
      des(block,bkey,method);
      StrFromBlock(text,block);
      fwrite(text,1,8,output_file);
    }
  }
  return 0;
}
