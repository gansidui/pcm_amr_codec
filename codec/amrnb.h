#ifndef __AMRNB_H__
#define __AMRNB_H__

#ifdef __cplusplus
extern "C" {
#endif

int amrnb_decode_init();
int amrnb_decode_uninit();
int amrnb_read_bytes(int nMode);
int amrnb_decode(char *pData, int nSize, FILE *fp);
int amrnb_decode_buf(char *pData, int nSize, char *pOutput);

int amrnb_encode_init(int nMode);
int amrnb_encode_uninit();
int amrnb_encode(char *pData, int nSize, FILE *fp);
int amrnb_encode_buf(char *pData, int nSize, char *pOutput);

#ifdef __cplusplus
}
#endif

#endif
