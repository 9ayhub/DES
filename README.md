# DES算法c++实现

**模块划分**：

（1）useDes模块：顶层模块

（2）IP_trans模块：初始置换IP

（3）Split_data模块：分解IP置换后的数据，获取 L0 和 R0

（4）T模块：十六轮迭代(获取L16,R16)

- F模块：Feistel轮函数，接收32位数据和48位子密钥，产生一个32位的输出

- generateKeys模块：生成16个48位的子密钥

- leftShift模块：对56位密钥的前后部分进行左移

（5）Merge模块：合并L16和R16，注意合并为 R16L16

（6）IP_1_trans模块：结尾置换IP-1


**参考资料**：

DES算法实例详解：http://www.hankcs.com/security/des-algorithm-illustrated.html

DES加密算法的C++实现：https://songlee24.github.io/2014/12/06/des-encrypt/

