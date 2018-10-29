#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
using namespace std;

bitset<64> key;                // 64位密钥
bitset<48> subKey[16];         // 存放16轮子密钥

/**********************************************************************/
/*                                                                    */
/*                          DES算法所用表                             */
/*                                                                    */
/**********************************************************************/

/*------------------初始、结尾置换所用表-----------------*/

// 初始置换表
int IP[] = {58, 50, 42, 34, 26, 18, 10, 2,
			60, 52, 44, 36, 28, 20, 12, 4,
			62, 54, 46, 38, 30, 22, 14, 6,
			64, 56, 48, 40, 32, 24, 16, 8,
			57, 49, 41, 33, 25, 17, 9,  1,
			59, 51, 43, 35, 27, 19, 11, 3,
			61, 53, 45, 37, 29, 21, 13, 5,
			63, 55, 47, 39, 31, 23, 15, 7};

// 结尾置换表
int IP_1[] = {40, 8, 48, 16, 56, 24, 64, 32,
			  39, 7, 47, 15, 55, 23, 63, 31,
			  38, 6, 46, 14, 54, 22, 62, 30,
			  37, 5, 45, 13, 53, 21, 61, 29,
			  36, 4, 44, 12, 52, 20, 60, 28,
			  35, 3, 43, 11, 51, 19, 59, 27,
			  34, 2, 42, 10, 50, 18, 58, 26,
			  33, 1, 41,  9, 49, 17, 57, 25};

/*------------------密钥调度所用表-----------------*/

// 密钥置换表，将64位密钥变成56位
int PC_1[] = {57, 49, 41, 33, 25, 17, 9,
			   1, 58, 50, 42, 34, 26, 18,
			  10,  2, 59, 51, 43, 35, 27,
			  19, 11,  3, 60, 52, 44, 36,
			  63, 55, 47, 39, 31, 23, 15,
			   7, 62, 54, 46, 38, 30, 22,
			  14,  6, 61, 53, 45, 37, 29,
			  21, 13,  5, 28, 20, 12,  4}; 

// 压缩置换，将56位密钥压缩成48位子密钥
int PC_2[] = {14, 17, 11, 24,  1,  5,
			   3, 28, 15,  6, 21, 10,
			  23, 19, 12,  4, 26,  8,
			  16,  7, 27, 20, 13,  2,
			  41, 52, 31, 37, 47, 55,
			  30, 40, 51, 45, 33, 48,
			  44, 49, 39, 56, 34, 53,
			  46, 42, 50, 36, 29, 32};

// 每轮左移的位数
int shiftBits[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

/*------------------下面是密码函数 f 所用表-----------------*/

// 扩展置换表，将 32位 扩展至 48位
int E[] = {32,  1,  2,  3,  4,  5,
		    4,  5,  6,  7,  8,  9,
		    8,  9, 10, 11, 12, 13,
		   12, 13, 14, 15, 16, 17,
		   16, 17, 18, 19, 20, 21,
		   20, 21, 22, 23, 24, 25,
		   24, 25, 26, 27, 28, 29,
		   28, 29, 30, 31, 32,  1};

// S盒，每个S盒是4x16的置换表，6位 -> 4位
int S_BOX[8][4][16] = {
	{  
		{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},  
		{0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},  
		{4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0}, 
		{15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13} 
	},
	{  
		{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},  
		{3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5}, 
		{0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},  
		{13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}  
	}, 
	{  
		{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},  
		{13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},  
		{13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},  
		{1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}  
	}, 
	{  
		{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},  
		{13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},  
		{10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},  
		{3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}  
	},
	{  
		{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},  
		{14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},  
		{4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},  
		{11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}  
	},
	{  
		{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},  
		{10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},  
		{9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},  
		{4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}  
	}, 
	{  
		{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},  
		{13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},  
		{1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},  
		{6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}  
	}, 
	{  
		{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},  
		{1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},  
		{7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},  
		{2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}  
	} 
};

// P置换，32位 -> 32位
int P[] = {16,  7, 20, 21,
		   29, 12, 28, 17,
		    1, 15, 23, 26,
		    5, 18, 31, 10,
		    2,  8, 24, 14,
		   32, 27,  3,  9,
		   19, 13, 30,  6,
		   22, 11,  4, 25 };

/**********************************************************************/
/*                                                                    */
/*                            DES算法实现                             */
/*                                                                    */
/**********************************************************************/


//0.工具函数：将char字符数组转为二进制
bitset<64> charToBitset(const char s[8]) {
	bitset<64> bits;
	for(int i = 0; i < 8; ++i)
		for(int j = 0; j < 8; ++j)
			bits[i * 8 + j] = ((s[i] >> j) & 1);
	return bits;
}


//1.初始置换IP
void IP_trans(bitset<64>& currentBits, bitset<64>& plain) {
	for(int i = 0; i < 64; ++i)
		currentBits[i] = plain[IP[i] - 1];
}

//2.获取 Li 和 Ri
void split_data(bitset<64>& currentBits, bitset<32>& left, bitset<32>& right) {
	for(int i = 32; i < 64; ++i)
		left[i-32] = currentBits[i];
	for(int i = 0; i < 32; ++i)
		right[i] = currentBits[i];
}


//3.1.1.1 对56位密钥的前后部分进行左移
bitset<28> leftShift(bitset<28> k, int shift) {
	bitset<28> tmp = k;
	for(int i = 0; i < 28; i++) {
		k[i] = tmp[(i + shift) % 28];
	}
	return k;
}

//3.1.1 生成16个48位的子密钥
void generateKeys() {
	bitset<56> realKey;
	bitset<28> left;
	bitset<28> right;
	bitset<48> compressKey;
	
	// 去掉奇偶标记位，将64位密钥变成56位
	for (int i = 0; i < 56; ++i)
		realKey[i] = key[PC_1[i] - 1];
	
	// 生成子密钥，保存在 subKeys[16] 中
	for(int round = 0; round < 16; ++round) 
	{
		// 前28位与后28位
		for(int i = 0; i < 28; ++i)
			left[i] = realKey[i];
		for(int i = 28; i < 56; ++i)
			right[i - 28] = realKey[i];
		
		// 左移
		left = leftShift(left, shiftBits[round]);
		right = leftShift(right, shiftBits[round]);
		
		// 压缩置换，由56位得到48位子密钥
		for(int i = 0; i < 28; ++i)
			realKey[i] = left[i];
		for(int i = 28; i < 56; ++i)
			realKey[i] = right[i-28];
		
		for(int i = 0; i < 48; ++i)
			compressKey[i] = realKey[PC_2[i] - 1];
		
		subKey[round] = compressKey;
	}
}

//3.1 密码函数f，接收32位数据和48位子密钥，产生一个32位的输出  
bitset<32> f(bitset<32> R, bitset<48> k) {
	bitset<48> expandR;
	
	// 第一步：扩展置换，32 -> 48
	for(int i = 0; i < 48; ++i)
		expandR[i] = R[E[i] - 1];
		
	// 第二步：异或
	expandR = expandR ^ k;
	
	// 第三步：查找S_BOX置换表
	bitset<32> output;
	for(int i = 0; i < 8; i++) {
		int row = expandR[i * 6] * 2 + expandR[i * 6 + 5] * 1; 
		int col = expandR[i * 6 + 1] * 8 + expandR[i * 6 + 2] * 4 + expandR[i * 6 + 3] * 2+ expandR[i * 6 + 4] * 1;
		int Si_r_c = S_BOX[i][row][col];
		
		bitset<4> b(Si_r_c);
		for(int j = 0; j < 4; j++) {
			output[i * 4 + j] = b[j];
		}
	}
	
	// 第四步：P-置换，32 -> 32
	bitset<32> tmp = output;
	for(int i = 0; i < 32; ++i)
		output[i] = tmp[P[i] - 1];
	
	return output;
}

//3.十六轮迭代(获取L16,R16)
void T(bitset<32>& right, bitset<32>& left, bool isEncode) {
	for(int round = 0; round < 16; ++round) {
		bitset<32> Ln = right;
		int keyNum = isEncode ? round : 15 - round;
		bitset<32> Rn = left ^ f(right,subKey[keyNum]);
		left = Ln;
		right = Rn;
	}
}


//4.合并L16和R16，注意合并为 R16L16
void merge(bitset<32>& left, bitset<32>& right, bitset<64>& cipher) {
	for(int i = 0; i < 32; ++i)
		cipher[i] = left[i];
	for(int i = 32; i < 64; ++i)
		cipher[i] = right[i-32];
}

//5.结尾置换IP-1
void IP_1_trans(bitset<64> currentBits, bitset<64>& cipher) {
	for(int i = 0; i < 64; ++i)
		cipher[i] = currentBits[IP_1[i] - 1];
}


/**
 *  DES(加密&解密)
 */
bitset<64> useDes(bitset<64>& plain, bool isEncode) {
	bitset<64> currentBits;
	bitset<32> left;
	bitset<32> right;
	bitset<64> cipher;
	
	// 第一步：初始置换IP
	IP_trans(currentBits, plain);
		
	// 第二步：获取 Li 和 Ri
	split_data(currentBits, left, right);
		
	// 第三步：共16轮迭代(获取L16,R16)
	T(right, left, isEncode);
	
	// 第四步：合并L16和R16，注意合并为 R16L16
	merge(left, right, cipher);
		
	// 第五步：结尾置换IP-1
	IP_1_trans(cipher, cipher);
	
	// 返回密文
	return cipher;
}

/**************************************************************************/
/*																		  */
/*测试：																  */		
/*	从键盘读取数据，存入文件1，再加密，存入文件2，然后将解密内容存入文件3 */ 
/*																		  */
/**************************************************************************/

int main() {
	//准备密钥 
	cout << "输入密钥："; 
	char k[16]; //64位 
    cin.getline(k, 16);
	key = charToBitset(k);
	generateKeys();   // 生成16个子密钥*/
	
	//获取数据 
    /*cout << "AI: What's your secret ?" << endl;
	cout << "Bob: Hmm..." << endl;
	cout << "Bob: ";*/
	cout << "输入明文："; 
    char data[160]; //确保位数为64的倍数（也可以通过补零) 
    cin.getline(data, 160);
    
	//将数据写入文件 originalData.txt 
    ofstream outfile;
    outfile.open("C:/Users/Sandman/Desktop/originalData.txt");
    outfile << data << endl;
    outfile.close();
    
    //读取文件originalData.txt中的数据， 将加密后的密文写入 encode.txt 
    ifstream in;
    in.open("C:/Users/Sandman/Desktop/originalData.txt", ios::binary);
	ofstream out;
	out.open("C:/Users/Sandman/Desktop/encode.txt", ios::binary);
	bitset<64> plain;
	/*cout << "AI: I'll keep your secret. If anyone else want to know it , they will just see this: " << endl;
	cout << "AI: (see the encode.txt)" << endl;*/
	cout << "\n加密数据已存入encode.txt\n" << endl; 
	while(in.read((char*)&plain, sizeof(plain)))
	{
		//加密 
		bitset<64> cipher = useDes(plain, true);
		//写入 encode.txt 
		out.write((char*)&cipher, sizeof(cipher));
	}
	in.close();
	out.close();
	
	// 读取 encode.txt中的密文，解密到 decode.txt
	in.open("C:/Users/Sandman/Desktop/encode.txt", ios::binary);
	out.open("C:/Users/Sandman/Desktop/decode.txt", ios::binary);
	bitset<64> cipher;
	/*cout << "\nBob: Oh, you're so sweet..." << endl;
	cout << "AI: Thx.And did you say \"";*/
	while(in.read((char*)&cipher, sizeof(cipher)))
	{
		bitset<64> plain  = useDes(cipher, false);
		out.write((char*)&plain, sizeof(plain));
	}
	//cout << "(see the decode.txt)\" ? " << endl;
	cout << "\n解密数据已存入decode.txt\n" << endl;
	in.close();
	out.close();
	//cout << "Bob: Yeah. And..." << endl;
	
	return 0;
}
