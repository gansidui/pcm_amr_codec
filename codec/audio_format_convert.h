#ifndef __AUDIO_FORMAT_CONVERT_H__
#define __AUDIO_FORMAT_CONVERT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* 文件格式互转:
* 返回0表示成功
*/
int amr2pcm(const char *szInputFileName, const char *szOutputFileName, int nMode);

int pcm2amr(const char *szInputFileName, const char *szOutputFileName, int nMode);



/**
* 将buffer转换为amr文件
*/

// 打开amr输出文件，返回文件句柄
FILE* amr_encode_open(const char *szOutputFileName, int nMode);

// 将pInputData转换成amr格式的数据追加到文件后面，
// 返回转换后的数据大小，若小于0 则表示不能继续追加数据，此时应该调用 amr_encode_close 关闭。
// 待转换的pInputData大小(nSize) 必须为320。（调用者可以将数据缓存到320再转换）
int amr_encode_append_data(char *pInputData, int nSize, FILE *fpOutput);

// 关闭
void amr_encode_close(FILE *fpOutput);



/**
* 将amr文件转换为pcm buffer
*/

// 打开amr输入文件，返回文件句柄
FILE* amr_decode_open(const char *szInputFileName, int nMode);

// pOutputData 为amr转换为pcm buffer的输出缓冲区，可写大小至少为320
// 返回pcm buffer写入的字节数， 若小于0，则应该调用 amr_decode_close 关闭。
int amr_decode_convert(FILE *fpInput, char *pOutputData);

// 关闭
void amr_decode_close(FILE *fpInput);



/**
*  pcm buffer --> amr buffer
*/

// 初始化
void buffer_pcm2amr_init(int nMode);

// 将 pInputData 转换为 amr格式输出到 pOutputData，返回转换后的数据大小，若小于0 则表示表示转换失败。
// 待转换的pInputData大小(nSize) 必须为320。（调用者可以将数据缓存到320再转换）
int buffer_pcm2amr_encode(char *pInputData, int nSize, char *pOutputData);

// 关闭
void buffer_pcm2amr_uninit();



/**
*  amr buffer --> pcm buffer
*/

// 初始化，返回每次转换amr buffer 的数据大小（amr每帧长度，根据nMode计算得出，最大为32）
int buffer_amr2pcm_init(int nMode);

// pOutputData 为amr转换为pcm buffer的输出缓冲区，可写大小至少为320
// 返回pcm buffer写入的字节数， 若小于0 则表示表示转换失败。
// 待转换的pInputData的大小(nSize)必须为 buffer_amr2pcm_init 的返回值
int buffer_amr2pcm_decode(char *pInputData, int nSize, char *pOutputData);

// 关闭
void buffer_amr2pcm_uninit();



#ifdef __cplusplus
}
#endif

#endif
