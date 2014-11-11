#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include "amrnb.h"


int amr2pcm(const char *szInputFileName, const char *szOutputFileName, int nMode)
{
	int nAmrSize = amrnb_read_bytes(nMode) + 1;
	int nRet = 0, nSize = 0;
	unsigned char cData[32];
	FILE *fp = NULL, *fout = NULL;

	if (!(fp = fopen(szInputFileName, "rb"))) return -1;
	if (!(fout = fopen(szOutputFileName, "wb"))) return -1;
	
	const char szFileHeader[] = "#!AMR\n";
	amrnb_decode_init();
	nRet = fread(cData, (size_t)1, strlen(szFileHeader), fp);

	while(1)
	{
		// Read AMR Data
		memset(&cData, 0, sizeof(cData));
		nRet = fread(cData, (size_t)1, (size_t)nAmrSize, fp);
		
		if(nRet > 0)
		{
			nSize = nRet;
			nRet = amrnb_decode((char*)(cData), nSize, fout);
		}
		else
		{
			break;
		}
	}

	fclose(fout);
	fclose(fp);
	amrnb_decode_uninit();

	return 0;
}

int pcm2amr(const char *szInputFileName, const char *szOutputFileName, int nMode)
{
	int nRet = 0, nSize = 0;
	unsigned char cData[320];
	int nPcmSize = 320;
	FILE *fp = NULL, *fout = NULL;

	if (!(fp = fopen(szInputFileName, "rb"))) return -1;
	if (!(fout = fopen(szOutputFileName, "wb"))) return -1;

	const char szFileHeader[] = "#!AMR\n";
	amrnb_encode_init(nMode);
	nRet = fwrite(szFileHeader, (size_t)1, strlen(szFileHeader), fout);

	while(1)
	{
		// Read PCM Data
		memset(&cData, 0, sizeof(cData));
		nRet = fread(cData, (size_t)1, (size_t)nPcmSize, fp);
		
		if(nRet > 0)
		{
			nSize = nRet;
			nRet = amrnb_encode((char*)(cData), nSize, fout);
		}
		else
		{
			break;
		}
	}

	fclose(fout);
	fclose(fp);
	amrnb_encode_uninit();

	return 0;
}


FILE* amr_encode_open(const char *szOutputFileName, int nMode)
{
	FILE *fout = NULL;
	if (!(fout = fopen(szOutputFileName, "wb"))) return NULL;

	const char szFileHeader[] = "#!AMR\n";
	amrnb_encode_init(nMode);
	fwrite(szFileHeader, (size_t)1, strlen(szFileHeader), fout);

	return fout;
}

int amr_encode_append_data(char *pInputData, int nSize, FILE *fpOutput)
{
	if (nSize != 320) return -1;

	return amrnb_encode(pInputData, nSize, fpOutput);
}

void amr_encode_close(FILE *fpOutput)
{
	fclose(fpOutput);
	amrnb_encode_uninit();
}


int amr_decode_nAmrSize;

FILE* amr_decode_open(const char *szInputFileName, int nMode)
{
	amr_decode_nAmrSize = amrnb_read_bytes(nMode) + 1;

	FILE *fp = NULL;
	if (!(fp = fopen(szInputFileName, "rb"))) return NULL;
	
	const char szFileHeader[] = "#!AMR\n";
	amrnb_decode_init();
	
	unsigned char cData[32];
	fread(cData, (size_t)1, strlen(szFileHeader), fp);
	
	return fp;
}

int amr_decode_convert(FILE *fp, char *pOutputData)
{
	unsigned char cData[32] = {0};
	int nRet = fread(cData, (size_t)1, (size_t)amr_decode_nAmrSize, fp);

	if(nRet > 0)
	{
		return amrnb_decode_buf((char*)(cData), nRet, pOutputData);
	}

	return -1;
}

void amr_decode_close(FILE *fp)
{
	fclose(fp);
	amrnb_decode_uninit();
}

void buffer_pcm2amr_init(int nMode)
{
	amrnb_encode_init(nMode);
}

int buffer_pcm2amr_encode(char *pInputData, int nSize, char *pOutputData)
{
	if (nSize != 320) return -1;

	return amrnb_encode_buf(pInputData, nSize, pOutputData);
}

void buffer_pcm2amr_uninit()
{
	amrnb_encode_uninit();
}

int buffer_amr2pcm_init(int nMode)
{
	amr_decode_nAmrSize = amrnb_read_bytes(nMode) + 1;
	amrnb_decode_init();
	return amr_decode_nAmrSize;
}

int buffer_amr2pcm_decode(char *pInputData, int nSize, char *pOutputData)
{
	if (nSize != amr_decode_nAmrSize) return -1;

	return amrnb_decode_buf(pInputData, nSize, pOutputData);
}

void buffer_amr2pcm_uninit()
{
	amrnb_decode_uninit();
}

