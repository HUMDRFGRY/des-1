#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cstdlib>
#include "des.h"

using namespace std ;

// 声明文件指针

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

// 二进制010串
// 不够64位，8字节如何进行加密,构建数据块存在问题
// 如果str不够8字节，需要特殊处理
void BlockFromStr(Block & block , const char * str)
{
	for(size_t i = 0 ; i < block.size() ; ++i)
	{
    // 指针的好处，可以处理数据的每一个字节，每一位（也就是访问内存）
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
  //计算文件文件大小 
  fseek(input_file,0L,SEEK_END);//跳到文件尾
  file_size=ftell(input_file);
  cout<<"file_size: "<<file_size<<endl;
  fseek(input_file,0L,SEEK_SET);// 回到文件头
  // 获取数据块数目
  number_of_blocks=file_size/8+((file_size%8)?1:0);
  cout<< "number_of_blocks: " <<number_of_blocks<<endl;

  Block block,bkey; // 数据块和密钥
  BlockFromStr(bkey,argv[4]);// 获取密钥
  char text[8];
  char buffer[8]; // 分组数据
  unsigned short int padding = 8 -file_size%8; // 缺失字节
  cout<<"padding: "<<padding<<endl;
  // 进行加解密
  while(fread(buffer,1,8,input_file)){ // 一直读取直到EOF==0
    block_count++;//从第一个数据块开始处理
    // 处理最后一个数据块
    memset(text,0,8);
    if(block_count==number_of_blocks){
    cout<<"正在处理最后一个数据块"<<endl;
      //加密逻辑
      if(method==e){
        if(padding<8){ // 需要构造辅助序列
          // padding是一个状态变量
          memset((buffer+8-padding),(unsigned char)padding,padding);
        }
        BlockFromStr(block,buffer);
        des(block,bkey,method);
        // 这里应该将block写入输出文件
        StrFromBlock(text,block);
        fwrite(text,1,8,output_file);

        if(padding==8){
          memset(buffer,(unsigned char)padding,8);
          des(block,bkey,method);
          StrFromBlock(text,block);
          fwrite(buffer,1,8,output_file);
        }
      }else{
      // 解密逻辑,因为加密过程已经保证密文数据块是完整的8字节
        BlockFromStr(block,buffer);
        des(block,bkey,method);
        // 这里涉及到类型转换
        StrFromBlock(text,block);
        padding=text[7];
        // 这个padding不正确
        cout<<"解密的padding(buffer[7]) "<<text[7]<<endl;
        cout<<"解密的padding "<<padding<<endl;

        if(padding<8)
          fwrite(text,1,8-padding,output_file);
      }
      // 最后一位,由加密过程决定,还原加密过程中构造的辅助序列
      // 信息位，从padding 可以看出是否使用了辅助序列
      // padding == 8   无需使用辅助序列
      // padding < 8    使用padding将data_block填充为8字节

    }else{
      // 处理非最后一个数据块,直接进行加解密，无需构造
      BlockFromStr(block,buffer);
      des(block,bkey,method);
      StrFromBlock(text,block);
      // 写入输出文件
      fwrite(text,1,8,output_file);

    }

  }


  /* 重写开始
   *
	ifstream srcFile(argv[2]) ;//输入文件
	ofstream tarFile(argv[3]) ;//输出文件
	if(!srcFile || !tarFile){
    cout<<"打开文件失败"<<endl;
    exit(-1);
  }
	//文件打开失败

	// 计算文件大小
    long file_size ,number_of_blocks,block_count=0;
    srcFile.seekg(0,srcFile.end);
    size_t size = srcFile.tellg();
    number_of_blocks=size/8 + ((size%8)?1:0); srcFile.seekg(0,srcFile.beg);
    cout << "File size is : " << size << endl;
    cout << " Number of blocks is : " << number_of_blocks << endl;

	Block block , bkey ;//数据块和密钥
	BlockFromStr(bkey , argv[4]) ;//获取密钥
  cout<<"进行求余 ：" << size%8<<endl;
	unsigned short int padding = 8 - size%8;
	char buffer[8] ;
	cout<<"需要填充的字符数 : "<< padding <<endl;

	while(1)
	{
    // 处理最后一个数据段
    if(block_count==number_of_blocks){
    // 加密
      if(method=='e'){
         if(padding==8){
           srcFile.read(buffer,8);
           BlockFromStr(block,buffer);
           des(block,bkey,method);
           StrFromBlock(buffer,block);
         }

      }else{
      // 解密
      //
      }
    } else{
      // 正常处理非最终数据段

    }
  }
		memset(buffer , 0 , 8) ;//将8个字节置0
		srcFile.read(buffer , 8) ;// 每次从源中读取8个字节数据
		BlockFromStr(block , buffer) ;//构建数据块
		des(block , bkey , method) ;
		StrFromBlock(buffer , block) ;//获取运算后的数据
		tarFile.write(buffer , 8) ;//写入目标文件
		// 进行最后一次读取并跳出循环,需要构造一个8字节的序列
		if(srcFile.eof() && padding!=0){
			cout<<"到达文件尾部"<<endl;
			memset((buffer+8-padding),0,padding);
			BlockFromStr(block,buffer);
			des(block,bkey,method);
			StrFromBlock(buffer,block);
			tarFile.write(buffer,8-padding);
			break;
		}
	tarFile.close();
	srcFile.close();
  */
  //重写结束
	return 0;
}

// 加解密流程
// 最后一个数据块需要区分加密/解密单独处理
//
