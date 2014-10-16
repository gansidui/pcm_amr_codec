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
// 返回转换后的数据大小，若为 -1 表示不能继续追加数据，此时应该调用 amr_encode_close 关闭。
// 待转换的pInoutData大小(nSize) 必须为320。（调用者可以将数据缓存到320再转换）
int amr_encode_append_data(FILE *fp, char *pInputData, int nSize);

// 关闭
void amr_encode_close(FILE *fp);



/**
* 将amr文件转换为pcm buffer
*/

// 打开amr输入文件，返回文件句柄
FILE* amr_decode_open(const char *szInputFileName, int nMode);

// pOutputData 为amr转换为pcm buffer的输出缓冲区，可写大小nSize至少为320
// 返回pcm buffer写入的字节数， 若为-1，则应该调用 amr_decode_close 关闭。
int amr_decode_convert(FILE *fp, unsigned char *pOutputData, int nSize);

// 关闭
void amr_decode_close(FILE *fp);



#ifdef __cplusplus
}
#endif

#endif
