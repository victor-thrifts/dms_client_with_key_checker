// openssltest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <openssl/opensslv.h>
#include <openssl/rsa.h>
#include <openssl/des.h>
#include <openssl/pem.h>
#include <openssl/err.h>
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "libcryptoMDd.lib")
#else
#pragma comment(lib, "libcryptoMD.lib")
#endif // DEBUG


static const wchar_t *Base64Digits = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define CipherAlogthm 	  EVP_des_ede3_cbc

// ---- rsa非对称加解密 ---- //   
#define PUB_KEY_FILE "pubkey.pem"    // 公钥路径  
#define PRI_KEY_FILE "prikey.pem"    // 私钥路径  
#define KEY_LENGTH  128               // 密钥长度(字节)
//modulus in format char decimal;
const char sMODULUS[] = "168471656040682369067024842962822083298669539093986092465366127155720426564118469603013433851342957534348138682751573582625566160744115198034661810651722103097169587486100008061472372333707837160018287515262612680876274406409425481997533147443089198863298280210902750052071080616127252265650629071271842635449";
const char sPriExponent[] = "150901371025529646290988179678709460820582255579217413062879326125625226668399261526101203807643534082751795733019254351698826384607184496620698500219378204253214947180327085393185504812856350810804446056695865412260029797508072007855612286308169717837624067781580510271891701740762168471560451778085144503393";
const char sEXPONENT[] = "65537";
BIGNUM * modulus = BN_new();
BIGNUM * exponent = BN_new();
BIGNUM * priexpon = BN_new();

int Base64Encode(const char* pSrc, int nLenSrc, std::wostream& pDstStrm, int nLenDst)
{
	wchar_t pDst[4];
	int nLenOut = 0;

	while (nLenSrc > 0) {
		if (nLenDst < 4) return(0);
		int len = 0;
		char s1 = pSrc[len++];
		char s2 = (nLenSrc > 1) ? pSrc[len++] : 0;
		char s3 = (nLenSrc > 2) ? pSrc[len++] : 0;
		pSrc += len;
		nLenSrc -= len;

		//------------------ lookup the right digits for output
		pDst[0] = Base64Digits[(s1 >> 2) & 0x3F];
		pDst[1] = Base64Digits[(((s1 & 0x3) << 4) | ((s2 >> 4) & 0xF)) & 0x3F];
		pDst[2] = Base64Digits[(((s2 & 0xF) << 2) | ((s3 >> 6) & 0x3)) & 0x3F];
		pDst[3] = Base64Digits[s3 & 0x3F];

		//--------- end of input handling
		if (len < 3) {  // less than 24 src bits encoded, pad with '='
			pDst[3] = L'=';
			if (len == 1)
				pDst[2] = L'=';
		}

		nLenOut += 4;

		// write the data to a file
		pDstStrm.write(pDst, 4);

		nLenDst -= 4;
	}

	if (nLenDst > 0) *pDst = 0;

	return (nLenOut);
}


// 函数方法生成密钥对   
auto generateRSAKey(std::string strKey[2])->void
{				  
	int ret = -1;
	// 公私密钥对    
	size_t pri_len;
	size_t pub_len;
	char *pri_key = NULL;
	char *pub_key = NULL;

	BN_dec2bn(&modulus, sMODULUS);
	//BN_set_word(exponent, RSA_F4);
	BN_dec2bn(&exponent, sEXPONENT);
	BN_dec2bn(&priexpon, sPriExponent);

	BIGNUM *p = BN_new();
	BIGNUM *q = BN_new();
	BN_set_word(p, 0);
	BN_set_word(q, 0);

	BIGNUM *dmp1 = BN_new();
	BIGNUM *dmq1 = BN_new();
	BIGNUM *iqmp = BN_new();
	BN_set_word(dmp1, 0);
	BN_set_word(dmq1, 0);
	BN_set_word(iqmp, 0);

	//EVP_PKEY_CTX *evp_ctx = NULL;
	//evp_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
	//ret = EVP_PKEY_keygen_init(evp_ctx);
	//EVP_PKEY_CTX_set_rsa_keygen_bits(evp_ctx, KEY_LENGTH*8);
	//EVP_PKEY_CTX_set_rsa_keygen_pubexp(evp_ctx, exponent);
	//EVP_PKEY_CTX_set_rsa_padding(evp_ctx, RSA_PKCS1_PADDING);
	EVP_PKEY *pkey = EVP_PKEY_new();
	//ret = EVP_PKEY_keygen(evp_ctx, &pkey);

	RSA* rsa = RSA_new_method(NULL);
	//ENGINE_get_default_RSA();		

	// 生成密钥对 
	ret = RSA_set0_key(rsa, modulus, exponent, priexpon);
	RSA_set0_factors(rsa, p, q);
	RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp);
	ret = RSA_generate_key_ex(rsa, KEY_LENGTH*8, exponent, NULL);

	//EVP_PKEY_assign_RSA(pkey, rsa);
	EVP_PKEY_set1_RSA(pkey, rsa);

	const BIGNUM *n, *e, *d;
	RSA_get0_key(rsa, &n, &e, &d);
	cout << "N KEY: " << BN_bn2dec(n) << endl;
	cout << "E KEY: " << BN_bn2hex(e) << endl;
	cout << "D KEY: " << BN_bn2dec(d) << endl;

	BIO *pri = BIO_new(BIO_s_mem());
	BIO *pub = BIO_new(BIO_s_mem());

	//PEM_write_bio_RSAPrivateKey(pri, rsa, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_PrivateKey(pri, pkey, NULL, NULL, 0, NULL, NULL);
	//PEM_write_bio_RSAPublicKey(pub, rsa);
	PEM_write_bio_PUBKEY(pub, pkey);
	if (ret < 0) {
		ret = ERR_get_error();
		printf("key generation failed\n");
		//printf("%s\n", ERR_error_string(ERR_get_error(), (char *)crip));
	}

	// 获取长度    
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	// 密钥对读取到字符串
	pri_key = (char *)malloc(pri_len + 1);
	pub_key = (char *)malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';

	// 存储密钥对    
	strKey[0] = pub_key;
	strKey[1] = pri_key;

	// 存储到磁盘（这种方式存储的是begin rsa public key/ begin rsa private key开头的）  
	FILE *pubFile = fopen(PUB_KEY_FILE, "w");
	if (pubFile == NULL)
	{
		assert(false);
		return;
	}
	fputs(pub_key, pubFile);
	fclose(pubFile);

	FILE *priFile = fopen(PRI_KEY_FILE, "w");
	if (priFile == NULL)
	{
		assert(false);
		return;
	}
	fputs(pri_key, priFile);
	fclose(priFile);

	// 内存释放  
	BIO_free_all(pub);
	BIO_free_all(pri);
	free(pri_key);
	free(pub_key);

	//EVP_PKEY_CTX_free(evp_ctx);
	//EVP_PKEY_free(pkey);
	EVP_cleanup();
	//RSA_free(rsa);
}


vector<string> getRSAKey()
{
	vector<string> ret;
	ifstream infile(PUB_KEY_FILE, ios_base::in|ios_base::binary);
	ostringstream inbuf; inbuf << infile.rdbuf();
	string str(inbuf.str());
	ret.push_back(str);
	infile.close();
	inbuf.str("");

	infile.open(PRI_KEY_FILE, ios_base::in|ios_base::binary);
	inbuf << infile.rdbuf();
	str = inbuf.str();
	ret.push_back(str);
	infile.close();
	return ret;
}

//des对称加密
/********************************************************
DES加密原理:

DES 使用一个 56 位的密钥以及附加的 8 位奇偶校验位
（每组的第8位作为奇偶校验位），产生最大 64 位的分组大小。
这是一个迭代的分组密码，使用称为 Feistel 的技术，其中将
加密的文本块分成两半。使用子密钥对其中一半应用循环功能，
然后将输出与另一半进行“异或”运算；接着交换这两半，
这一过程会继续下去，但最后一个循环不交换。DES 使用 16
轮循环，使用异或，置换，代换，移位操作四种基本运算。
**********************************************************/
// ecb模式
string des_encrypt(string &text, const string &key)
{
	string cipherText; //密文
	DES_cblock keyEncrypt;
	memset(keyEncrypt, 0, 8);
	if (key.length() <= 8)   //构造补齐后的秘钥
	{
		memcpy(keyEncrypt, key.c_str(), key.length());
	}
	else
	{
		memcpy(keyEncrypt, key.c_str(), 8);
	}
	//密钥置换
	DES_key_schedule keySchedule;
	DES_set_key_unchecked(&keyEncrypt, &keySchedule); //设置密码表，不需要校验
	//循环加密，每8字节一次
	const_DES_cblock inputText;
	DES_cblock outputText;
	vector<unsigned char> vecCiphertext;
	unsigned char tmp[8] = { 0 };
	for (size_t i = 0; i<text.length() / 8; i++)
	{
		memcpy(inputText, text.c_str() + i * 8, 8);
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
		memcpy(tmp, outputText, 8);
		for (int j = 0; j<8; j++)
		{
			vecCiphertext.push_back(tmp[j]);
		}
	}
	if (text.length() % 8 != 0)
	{
		int tmp1 = text.length() / 8 * 8;
		int tmp2 = text.length() - tmp1;
		memset(inputText, 0, 8);
		memcpy(inputText, text.c_str() + tmp1, tmp2);
		//加密函数
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
		memcpy(tmp, outputText, 8);
		for (int i = 0; i<8; i++)
		{
			vecCiphertext.push_back(tmp[i]);
		}
	}
	cipherText.clear();
	cipherText.assign(vecCiphertext.begin(), vecCiphertext.end());
	return cipherText;
}

//解密ecb
string des_decrypt(const string &cipherText, const string &key)
{
	string clearText; //明文
	DES_cblock keyEncrypt;
	memset(keyEncrypt, 0, 8);
	if (key.length() <= 8)
	{
		memcpy(keyEncrypt, key.c_str(), key.length());
	}
	else
	{
		memcpy(keyEncrypt, key.c_str(), 8);
	}
	DES_key_schedule keyScheule;
	DES_set_key_unchecked(&keyEncrypt, &keyScheule); //设置密码表，不校验
	const_DES_cblock inputText;
	DES_cblock outputText;
	vector<unsigned char> vecClearText;
	unsigned char tmp[8];
	for (size_t i = 0; i<cipherText.length() / 8; i++)
	{
		memcpy(inputText, cipherText.c_str() + i * 8, 8);
		DES_ecb_encrypt(&inputText, &outputText, &keyScheule, DES_DECRYPT);
		memcpy(tmp, outputText, 8);
		for (int j = 0; j<8; j++)
		{
			vecClearText.push_back(tmp[j]);
		}
	}
	if (cipherText.length() % 8 != 0)
	{
		int tmp1 = cipherText.length() / 8 * 8;
		int tmp2 = cipherText.length() - tmp1;
		memset(inputText, 0, 8);
		memcpy(inputText, cipherText.c_str() + tmp1, tmp2);
		//解密函数
		DES_ecb_encrypt(&inputText, &outputText, &keyScheule, DES_DECRYPT);
		memcpy(tmp, outputText, 8);
		for (int i = 0; i<8; i++)
		{
			vecClearText.push_back(tmp[i]);
		}

	}
	clearText.clear();
	clearText.assign(vecClearText.begin(), vecClearText.end());
	return clearText;
}

// 私钥加密
std::string rsa_pri_encrypt(const std::string &clearText, std::string &sKey)
{
	std::string strRet;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)sKey.c_str(), -1);

	EVP_PKEY *evpkey = NULL;
	PEM_read_bio_PrivateKey(keybio, &evpkey, NULL, NULL);
	RSA* rsa = EVP_PKEY_get1_RSA(evpkey);
	//rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	//if (!rsa)
	//{
	//	BIO_free_all(keybio);
	//	return std::string("");
	//}

	//RSA* rsa1 = RSA_new_method(NULL);
	////ENGINE_get_default_RSA();	
	//BIGNUM * modulus1 = BN_new();
	//BIGNUM * exponent1 = BN_new();
	//BIGNUM * priexpon1 = BN_new();
	//RSA_get0_key(rsa, (const BIGNUM**)&modulus1, (const BIGNUM**)&exponent1, (const BIGNUM**)&priexpon1);
	//RSA_set0_key(rsa1, modulus1, exponent1, priexpon1);

	int len = RSA_size(rsa);
	char *encryptedText = (char *)malloc(len + 1);
	memset(encryptedText, 0, len + 1);

	// 加密  
	int ret = RSA_private_encrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);
	if (ret >= 0){
		strRet = std::string(encryptedText, ret);
	}
	else { char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err)); }
	// 释放内存  
	free(encryptedText);
	BIO_free_all(keybio);
	//RSA_free(rsa);
	EVP_cleanup();
	return strRet;
}

// 公钥解密    
std::string rsa_pub_decrypt(const std::string &encText, std::string &sKey)
{
	std::string strRet;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)sKey.c_str(), -1);

	EVP_PKEY* evpkey = NULL;
	PEM_read_bio_PUBKEY(keybio, &evpkey, NULL, NULL);
	RSA* rsa = EVP_PKEY_get1_RSA(evpkey);
	//rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);
	//if (!rsa)
	//{
	//	BIO_free_all(keybio);
	//	return std::string("");
	//}

	//RSA* rsa1 = RSA_new_method(NULL);
	////ENGINE_get_default_RSA();	
	//BIGNUM * modulus1 = BN_new();
	//BIGNUM * exponent1 = BN_new();
	//BIGNUM * priexpon1 = BN_new();
	//RSA_get0_key(rsa, (const BIGNUM**)&modulus1, (const BIGNUM**)&exponent1, NULL);
	//BN_set_word(priexpon1, 0);
	//RSA_set0_key(rsa1, modulus1, exponent1, priexpon1);

	int len = RSA_size(rsa);
	char *clearText = (char *)malloc(len + 1);
	memset(clearText, 0, len + 1);

	//解密
	int ret = RSA_public_decrypt(encText.length(), (const unsigned char*)encText.c_str(), (unsigned char*)clearText, rsa, RSA_PKCS1_PADDING);
	if (ret >= 0){ strRet = std::string(clearText, ret); }
	else{ char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err)); }


	// 释放内存  
	free(clearText);
	BIO_free_all(keybio);
	//RSA_free(rsa);
	EVP_cleanup();
	return strRet;
}

//私钥加密 + 分片
std::string rsa_pri_split117_encrypt(const std::string &clearText, std::string &sKey)
{
	std::string result;
	std::string input;
	result.clear();
	for (int i = 0; i < clearText.length() / 117; i++)
	{
		input.clear();
		input.assign(clearText.begin() + i * 117, clearText.begin() + i * 117 + 117);
		result = result + rsa_pri_encrypt(input, sKey);
	}
	if (clearText.length() % 117 != 0)
	{
		int tem1 = clearText.length() / 117 * 117;
		int tem2 = clearText.length() - tem1;
		input.clear();
		input.assign(clearText.begin() + tem1, clearText.end());
		result = result + rsa_pri_encrypt(input, sKey);
	}
	return result;
}

//公钥解密 + 分片
std::string rsa_pub_split128_decrypt(const std::string &encText, std::string &sKey)
{
	//Base64 *base = new Base64();
	std::string result;
	std::string input;
	result.clear();
	for (int i = 0; i< encText.length() / 128; i++)
	{
		input.clear();
		input.assign(encText.begin() + i * 128, encText.begin() + i * 128 + 128);

		result = result + rsa_pub_decrypt(input, sKey);
	}
	if (encText.length() % 128 != 0)
	{
		int tem1 = encText.length() / 128 * 128;
		int tem2 = encText.length() - tem1;
		input.clear();
		input.assign(encText.begin() + tem1, encText.end());
		result = result + rsa_pub_decrypt(input, sKey);
	}
	return result;
}

// 公钥加密，这里key必须是公钥
string rsa_pub_encrypt(string& orig_data, string &skey)
{	
	string ret;
	BIO *bp = BIO_new_mem_buf(skey.c_str(), skey.size());
	EVP_PKEY *key = nullptr;
	key = PEM_read_bio_PUBKEY(bp, &key, NULL, NULL);

	EVP_PKEY_CTX *ctx = NULL;	
	ctx = EVP_PKEY_CTX_new(key, NULL);	
	if(NULL == ctx)	{		
		printf("ras_pubkey_encryptfailed to open ctx.\n");		
		EVP_PKEY_free(key);		
		return ret;	
	} 	
	EVP_PKEY_encrypt_init(ctx);
	EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING);
#define KL (KEY_LENGTH)
	unsigned char enc_data[KEY_LENGTH]; size_t enc_data_len;
	std::string input;  
	int step = orig_data.size() / KL;
	for (int i = 0; i < step; i++)
	{
		input.clear();
		input.assign(orig_data.begin() + i * KL, orig_data.begin() + (i + 1) * KL);
		if (EVP_PKEY_encrypt(ctx, enc_data, &enc_data_len, (unsigned char*)input.c_str(), input.size()) <= 0)	{
			char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));
			printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt.\n");
			EVP_PKEY_CTX_free(ctx);
			EVP_PKEY_free(key);
			return "";
		}
		ret += string((char*)enc_data, enc_data_len);
	}
	if (step*KL < orig_data.size()){
		input.assign(orig_data.begin() + step*KL, orig_data.end());
		if (EVP_PKEY_encrypt(ctx, enc_data, &enc_data_len, (unsigned char*)input.c_str(), input.size()) <= 0)	{
			char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));
			printf("ras_pubkey_encryptfailed to EVP_PKEY_encrypt.\n");
			EVP_PKEY_CTX_free(ctx);
			EVP_PKEY_free(key);
			return "";
		}
		ret += string((char*)enc_data, enc_data_len);
	}
#undef KL
	EVP_PKEY_CTX_free(ctx);	
	EVP_PKEY_free(key); 	
	return ret;
}

//密钥解密，这种封装格式只适用公钥加密，私钥解密，这里key必须是私钥
string rsa_pri_decrypt(string &enc_data, string &skey, char *passwd)
{	
	string ret;
	EVP_PKEY *key = NULL;
	BIO* bp = BIO_new_mem_buf(skey.c_str(), skey.size());
	key = PEM_read_bio_PrivateKey(bp, &key, NULL, passwd);

	//RSA* rsa = EVP_PKEY_get1_RSA(key);
	//RSA* rsa1 = RSA_new_method(NULL);
	//BIGNUM * modulus1 = BN_new();
	//BIGNUM * exponent1 = BN_new();
	//BIGNUM * priexpon1 = BN_new();
	//RSA_get0_key(rsa, (const BIGNUM**)&modulus1, (const BIGNUM**)&exponent1, (const BIGNUM**)&priexpon1);
	//RSA_set0_key(rsa1, modulus1, exponent1, priexpon1);
	//EVP_PKEY_set1_RSA(key, rsa1);

	EVP_PKEY_CTX *ctx = NULL;
	ctx = EVP_PKEY_CTX_new(key, NULL);	
	EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_NO_PADDING);
	if(NULL == ctx)	{		
		printf("ras_prikey_decryptfailed to open ctx.\n");		
		EVP_PKEY_free(key);		
		return ret;
	} 	
	if(EVP_PKEY_decrypt_init(ctx) <= 0)	{		
		printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt_init.\n");		
		EVP_PKEY_free(key);		
		return ret;
	} 	
#define XL (KEY_LENGTH)
	unsigned char orig_data[KEY_LENGTH]; size_t orig_data_len;
	std::string input;
	int step = enc_data.size() / XL;
	for (int i = 0; i < step; i++)
	{
		input.clear();
		input.assign(enc_data.begin() + i * XL, enc_data.begin() + (i + 1) * XL);
		if (EVP_PKEY_decrypt(ctx, orig_data, &orig_data_len, (unsigned char*)input.c_str(), input.size()) <= 0) {
			char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));
			printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt.\n");
			EVP_PKEY_CTX_free(ctx);
			EVP_PKEY_free(key);
			return "";
		}
		ret += string((char*)orig_data, orig_data_len);
	}
	if (step*XL < enc_data.size()){
		input.assign(enc_data.begin() + step*XL, enc_data.end());
		if (EVP_PKEY_decrypt(ctx, orig_data, &orig_data_len, (unsigned char*)input.c_str(), input.size()) <= 0) {
			char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));
			printf("ras_prikey_decryptfailed to EVP_PKEY_decrypt.\n");
			EVP_PKEY_CTX_free(ctx);
			EVP_PKEY_free(key);
			return "";
		}
		ret += string((char*)orig_data, orig_data_len);
	}
#undef XL
	EVP_PKEY_CTX_free(ctx);	
	EVP_PKEY_free(key);	
	return ret;
}

 //对称加密
int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
	int ret, len, ciphertext_len;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	ret = EVP_EncryptInit_ex(ctx, CipherAlogthm(), NULL, key, iv);
	ret = EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
	ciphertext_len = len;
	ret = EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
	ciphertext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return ciphertext_len;
}

//对称解密
int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
	int ret, len, plaintext_len;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	ret = EVP_DecryptInit_ex(ctx, CipherAlogthm(), NULL, key, iv);
	ret = EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
	plaintext_len = len;
	ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
	plaintext_len += len;
	EVP_CIPHER_CTX_free(ctx);
	return plaintext_len;
}

//performs verification of a string using an HMAC.
int hmac_verify(const unsigned char* msg, size_t mlen, const unsigned char* sig, size_t slen, unsigned char* pkey)
{
	int result = -1;
	if (!msg || !mlen || !sig || !slen || !pkey) return -1;

	EVP_PKEY *evpkey = EVP_PKEY_new();
	BIO *bp = BIO_new(BIO_s_mem());
	BIO_read(bp, (void*)pkey, strlen((const char*)pkey));
	evpkey = PEM_read_bio_PrivateKey(bp, &evpkey, NULL, NULL);

	EVP_MD_CTX* ctx = EVP_MD_CTX_create();
	do
	{
		const EVP_MD* md = EVP_get_digestbyname("SHA1");
		int rc = EVP_DigestInit_ex(ctx, md, NULL);
		rc = EVP_DigestSignInit(ctx, NULL, md, NULL, evpkey);
		rc = EVP_DigestSignUpdate(ctx, msg, mlen);
		unsigned char buff[EVP_MAX_MD_SIZE];
		size_t req = sizeof(buff);
		rc = EVP_DigestSignFinal(ctx, buff, &req);
		const size_t m = (slen < req ? slen : req);
		result = !CRYPTO_memcmp(sig, buff, m);
		OPENSSL_cleanse(buff, sizeof(buff));
	} while (0);
	if (ctx) {
		EVP_MD_CTX_destroy(ctx);
		ctx = NULL;
	}
	return !!result;
}

//Asymmetric Key verify
int verify(const unsigned char* msg, size_t mlen, const unsigned char* sig, size_t slen, unsigned char* pkey)
{
	int result = -1;
	if (!msg || !mlen || !sig || !slen || !pkey) return result;

	EVP_PKEY *evpkey = EVP_PKEY_new();
	BIO *bp = BIO_new(BIO_s_mem());
	BIO_write(bp, pkey, strlen((char*)pkey));
	evpkey = PEM_read_bio_PUBKEY(bp, &evpkey, NULL, NULL);

	EVP_MD_CTX* ctx = EVP_MD_CTX_create();
	do
	{
		const EVP_MD* md = EVP_get_digestbyname("SHA1");
		int rc = EVP_DigestInit_ex(ctx, md, NULL);
		rc = EVP_DigestVerifyInit(ctx, NULL, md, NULL, evpkey);
		rc = EVP_DigestVerifyUpdate(ctx, msg, mlen);
		/* Clear any errors for the call below */
		ERR_clear_error();
		rc = EVP_DigestVerifyFinal(ctx, sig, slen);
		char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));
		result = 1;
	} while (0);
	if (ctx) {
		EVP_MD_CTX_destroy(ctx);
		ctx = NULL;
	}
	return !!result;
}

//Asymmetric Key sign   OR     hmac  sign;
int sign(const unsigned char* msg, size_t mlen, unsigned char** sig, size_t* slen, unsigned char* pkey)
{
	int result = -1;   *slen = 0;
	if (!msg || !mlen || !sig || !pkey) return result;
	if (*sig) OPENSSL_free(*sig);  *sig = NULL;

	EVP_PKEY *evpkey = EVP_PKEY_new();
	BIO *bp = BIO_new(BIO_s_mem());
	BIO_write(bp, pkey,strlen((char*)pkey));
	//BIO *bp = BIO_new_mem_buf(pkey, -1);

	evpkey = PEM_read_bio_PrivateKey(bp, &evpkey, NULL, NULL);
	char err[128]; printf("%s\n", ERR_error_string(ERR_get_error(), err));

	EVP_MD_CTX* ctx = EVP_MD_CTX_create();
	do
	{
		const EVP_MD* md = EVP_get_digestbyname("SHA1");  //EVP_sha1()
		int rc = EVP_DigestInit_ex(ctx, md, NULL);
		rc = EVP_DigestSignInit(ctx, NULL, md, NULL, evpkey);
		rc = EVP_DigestSignUpdate(ctx, msg, mlen);
		size_t req = 0;
		rc = EVP_DigestSignFinal(ctx, NULL, &req);
		*sig = (unsigned char*)OPENSSL_malloc(req);
		*slen = req;
		rc = EVP_DigestSignFinal(ctx, *sig, slen);
		result = 1;
	} while (0);
	if (ctx) {
		EVP_MD_CTX_destroy(ctx);
		ctx = NULL;
	}
	return !!result;
}

int main1()
{
	string keypaire[2];
	//generateRSAKey(keypaire);
	vector<string> rr = getRSAKey();
	keypaire[0] = rr[0];
	keypaire[1] = rr[1];

	ifstream infile("zzfwserver.txt", ios_base::in|ios_base::binary);
	ostringstream sbuf; sbuf << infile.rdbuf();
	string plaintext(sbuf.str());
	string out;

	out = rsa_pri_split117_encrypt(plaintext, keypaire[1]);

	ofstream outfile;
	outfile.open("ZZFWServer.lic", ios_base::ate|ios_base::out|ios_base::binary);
	istringstream sbuf1; sbuf1.str(out);
	sbuf1 >> outfile.rdbuf();
	outfile.close();

	out = rsa_pub_split128_decrypt(out, keypaire[0]);


	unsigned char *key = (unsigned char *) "key";
	/* A 128 bit IV  Should be hardcoded in both encrypt and decrypt. */
	unsigned char *iv = (unsigned char *)"iv";
	unsigned char ciphertext[KEY_LENGTH*8], base64_in[KEY_LENGTH*8], base64_out[KEY_LENGTH*8];
	/* Buffer for the decrypted text */
	unsigned char decryptedtext[KEY_LENGTH*8];
	int decryptedtext_len, ciphertext_len;
	/* Encrypt the plaintext */
	ciphertext_len = encrypt((unsigned char*)plaintext.c_str(), plaintext.size(), key, iv, ciphertext);
	int encode_str_size = EVP_EncodeBlock(base64_out, ciphertext, ciphertext_len);
	/* Decrypt the plaintext */
	memcpy(base64_in, base64_out, KEY_LENGTH*8);
	ciphertext_len = encode_str_size;
	int length = EVP_DecodeBlock(base64_out, base64_in, ciphertext_len);
	while (base64_in[--ciphertext_len] == '=') length--;
	decryptedtext_len = decrypt(base64_out, length, key, iv, decryptedtext);
	decryptedtext[decryptedtext_len] = '\0';

	/* signing and verify */
	unsigned char* sig = NULL; size_t slen = 0;
	sign((unsigned char*)plaintext.c_str(), plaintext.size(), &sig, &slen, (unsigned char*)keypaire[1].c_str());
	//memset((char*)plaintext.c_str(), 0, plaintext.capacity());
	verify((unsigned char*)plaintext.c_str(), plaintext.size(), sig, slen, (unsigned char*)keypaire[0].c_str());

	plaintext = rsa_pub_encrypt(plaintext, keypaire[0]);
	plaintext = rsa_pri_decrypt(plaintext, keypaire[1], NULL);
	/* Clean up */
	EVP_cleanup();
	ERR_free_strings();
    return 0;
}


int main()
{
	int ret = -1;
	string keypaire[2];
	vector<string> rr = getRSAKey();
	keypaire[0] = rr[0];
	keypaire[1] = rr[1];

	ifstream infile("dms.lic", ios_base::in | ios_base::binary);
	infile.seekg(0, ios_base::end);
	int len = infile.tellg();
	infile.seekg(ios_base::beg);
	char * buffer = new char[len];
	infile.read(buffer, len);
	string out;
	out = rsa_pub_split128_decrypt(string(buffer,len), keypaire[0]);
	size_t pos = out.find("\n\r\n\r");
	string firstEncoderData(out.begin(), out.begin()+pos);
	string secondSignData(out.begin() + pos+4, out.end());
	ret = verify((unsigned char*)firstEncoderData.c_str(), firstEncoderData.size(), (unsigned char*)secondSignData.c_str(), secondSignData.size(), (unsigned char*)keypaire[0].c_str());
	if (!ret) return ret;
	out = rsa_pub_split128_decrypt(firstEncoderData, keypaire[0]);

	return ret;
}

