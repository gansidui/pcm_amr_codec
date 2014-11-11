#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include "audio_format_convert.h"


int main(int argc, char *argv[])
{
	// szInputFileName 为 amr 文件
	// szOutputFileName 为 pcm 文件

	char szInputFileName[256] = {0};
	char szOutputFileName[256] = {0};

	if(argc <= 1)
	{
		printf("No Params\n");
		exit(0);
	}

	strcpy(szInputFileName, argv[1]);
	strcpy(szOutputFileName, basename(szInputFileName));
	strcat(szOutputFileName, ".pcm");


	// 直接转换文件
/*
	if (!amr2pcm(szInputFileName, szOutputFileName, 1))
	{
		printf("Succeed\n");
	}
	else
	{
		printf("Failed\n");
	}
*/	

	
	// 将amr一段一段的转换为pcm

	FILE *fp = amr_decode_open(szInputFileName, 1);
	FILE *fout = fopen(szOutputFileName, "wb");

	if (!fout || !fp) return -1;

	unsigned char pOutputData[320];
	int nRet = 0;	
	while (1)
	{
		nRet = amr_decode_convert(fp, pOutputData);
		if (nRet > 0)
		{
			// pOutputData[0] -- pOutputData[nRet-1] 这段buffer为转换后的pcm流
			fwrite(pOutputData, 1, nRet, fout);
		}
		else
		{	
			amr_decode_close(fp);
			break;
		}
		
	}
	fclose(fout);


	return 0;
}
