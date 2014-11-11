#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include "audio_format_convert.h"


int main(int argc, char *argv[])
{
	// szInputFileName 为 pcm 文件
	// szOutputFileName 为 amr 文件

	char szInputFileName[256] = {0};
	char szOutputFileName[256] = {0};

	if(argc <= 1)
	{
		printf("No Params\n");
		exit(0);
	}

	strcpy(szInputFileName, argv[1]);
	strcpy(szOutputFileName, basename(szInputFileName));
	strcat(szOutputFileName, ".amr");


	// 直接转换文件
/*
	if (!pcm2amr(szInputFileName, szOutputFileName, 1))
	{
		printf("Succeed\n");
	}
	else
	{
		printf("Failed\n");
	}
*/


	// 将pcm一段一段的转换为amr

	FILE *fp = amr_encode_open(szOutputFileName, 1);
	FILE *fin = fopen(szInputFileName, "rb");

	if (!fin || !fp) return -1;

	unsigned char pInputData[320];
	int nRet = 0;	
	while (1)
	{
		nRet = fread(pInputData, 1, 320, fin);
		if (320 == nRet)
		{
			nRet = amr_encode_append_data(pInputData, 320, fp);
			if (nRet < 0)
			{
				amr_encode_close(fp);
				break;
			}
		}
		else
		{
			amr_encode_close(fp);
			break;
		}	
	}
	fclose(fin);


	return 0;
}
