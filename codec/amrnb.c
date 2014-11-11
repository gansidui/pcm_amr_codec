#include <stdio.h>
#include <stdlib.h>
#include <opencore-amrnb/interf_dec.h>
#include <opencore-amrnb/interf_enc.h>
#include "bs.h"
#include "amrnb.h"

#define MAX_FRAME_TYPE	(8)		// SID Packet
#define OUT_MAX_SIZE	(32)
#define NUM_SAMPLES 	(160)

static const int amr_frame_rates[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200};

static const int amr_frame_sizes[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0 };

void *amrnb_dec = NULL;
void *amrnb_enc = NULL;
int b_octet_align = 1;
int amrnb_dtx = 0;
int amrnb_ptime = 20;
int amrnb_mode;


#define toc_get_f(toc) ((toc) >> 7)
#define toc_get_index(toc)	((toc>>3) & 0xf)

static int toc_list_check(uint8_t *tl, size_t buflen) 
{
	int s = 1;
	while (toc_get_f(*tl))
	{
		tl++;
		s++;
		if (s > buflen)
		{
			return -1;
		}
	}
	return s;
}

int amrnb_decode_init()
{
	amrnb_dec = Decoder_Interface_init();
	return 0;
}

int amrnb_decode_uninit()
{
	Decoder_Interface_exit(amrnb_dec);
	amrnb_dec = NULL;
	return 0;
}

int amrnb_read_bytes(int nMode)
{
	if( (nMode >= 0) && (nMode < MAX_FRAME_TYPE) )
	{
		return amr_frame_sizes[nMode];
	}
	return 0;
}

int amrnb_decode(char *pData, int nSize, FILE *fp)
{
	int nRet = 0;
	static const int nsamples = NUM_SAMPLES;
	uint8_t tmp[OUT_MAX_SIZE];
	uint8_t output[NUM_SAMPLES * 2];
	
	uint8_t	tocs[20] = {0,};
	int 	nTocLen = 0, toclen = 0;
	bs_t	*payload = NULL;
	int		nCmr = 0, nBitLeft = 0, nPadding = 0, nReserved = 0, nRead = 0;
	int		nFbit = 1;
	int		nFTbits = 0;
	int		nQbit = 0;
	int		nFrameData = 0;
	int		i = 0, index = 0, framesz = 0;
	
	if(nSize < 2)
	{
		printf("Too short packet\n");
		return -1;
	}
	
	payload = bs_new((uint8_t *)pData, nSize);
	if(payload == NULL)
	{
		return -2;
	}

	nTocLen = 0; nFrameData = 0;
	while(nFbit == 1)
	{	// 0                   1
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |1|  FT   |Q|1|  FT   |Q|0|  FT   |Q|
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		nFbit = bs_read_u(payload, 1);
		nFTbits = bs_read_u(payload, 4);
		if(nFTbits > MAX_FRAME_TYPE)
		{
			printf("%s, Bad amr toc, index=%i (MAX=%d)\n", __func__, nFTbits, MAX_FRAME_TYPE);
			break;
		}
		nFrameData += amr_frame_sizes[nFTbits];
		nQbit = bs_read_u(payload, 1);
		tocs[nTocLen++] = ((nFbit << 7) | (nFTbits << 3) | (nQbit << 2)) & 0xFC;
		if(b_octet_align == 1)
		{	// octet-align 모드에서는 Padding bit 2bit를 더 읽어야 한다.
			nPadding = bs_read_u(payload, 2);
		}
		
	} // end of while

	nBitLeft = payload->bits_left;
	toclen = toc_list_check(tocs, nSize);

	if (toclen == -1)
	{
		printf("Bad AMR toc list\n");
		bs_free(payload);
		return -3;
	}
	
	if((nFrameData) != bs_bytes_left(payload))
	{
		printf("%s, invalid data mismatch, FrameData=%d, bytes_left=%d\n", __func__, nFrameData, bs_bytes_left(payload));
	}
	// printf("nTocLen==============%d\n", nTocLen);
	for(i=0; i<nTocLen; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		tmp[0] = tocs[i];
		index = toc_get_index(tocs[i]);
		if (index > MAX_FRAME_TYPE)
		{
			printf("Bad amr toc, index=%i\n", index);
			break;
		}

		framesz = amr_frame_sizes[index];
		nRead = bs_read_bytes_ex(payload, &tmp[1], framesz);
		nSize = nsamples * 2;
		
		Decoder_Interface_Decode(amrnb_dec, tmp, (short*) output, 0);
		nRet = fwrite(output, (size_t)1, nSize, fp);
		// printf("nRet == %d, nSize == %d\n", nRet, nSize);

	} // end of for
	bs_free(payload);

	return nRet;
}

// same as amrnb_decode
int amrnb_decode_buf(char *pData, int nSize, /*FILE *fp*/ char *pOutput)
{
	int nRet = 0;
	static const int nsamples = NUM_SAMPLES;
	uint8_t tmp[OUT_MAX_SIZE];
	uint8_t output[NUM_SAMPLES * 2];
	
	uint8_t	tocs[20] = {0,};
	int 	nTocLen = 0, toclen = 0;
	bs_t	*payload = NULL;
	int		nCmr = 0, nBitLeft = 0, nPadding = 0, nReserved = 0, nRead = 0;
	int		nFbit = 1;
	int		nFTbits = 0;
	int		nQbit = 0;
	int		nFrameData = 0;
	int		i = 0, index = 0, framesz = 0;
	
	if(nSize < 2)
	{
		printf("Too short packet\n");
		return -1;
	}
	
	payload = bs_new((uint8_t *)pData, nSize);
	if(payload == NULL)
	{
		return -2;
	}

	nTocLen = 0; nFrameData = 0;
	while(nFbit == 1)
	{	// 0                   1
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		// |1|  FT   |Q|1|  FT   |Q|0|  FT   |Q|
		// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		nFbit = bs_read_u(payload, 1);
		nFTbits = bs_read_u(payload, 4);
		if(nFTbits > MAX_FRAME_TYPE)
		{
			printf("%s, Bad amr toc, index=%i (MAX=%d)\n", __func__, nFTbits, MAX_FRAME_TYPE);
			break;
		}
		nFrameData += amr_frame_sizes[nFTbits];
		nQbit = bs_read_u(payload, 1);
		tocs[nTocLen++] = ((nFbit << 7) | (nFTbits << 3) | (nQbit << 2)) & 0xFC;
		if(b_octet_align == 1)
		{	// octet-align 모드에서는 Padding bit 2bit를 더 읽어야 한다.
			nPadding = bs_read_u(payload, 2);
		}
		
	} // end of while

	nBitLeft = payload->bits_left;
	toclen = toc_list_check(tocs, nSize);

	if (toclen == -1)
	{
		printf("Bad AMR toc list\n");
		bs_free(payload);
		return -3;
	}
	
	if((nFrameData) != bs_bytes_left(payload))
	{
		printf("%s, invalid data mismatch, FrameData=%d, bytes_left=%d\n", __func__, nFrameData, bs_bytes_left(payload));
	}
	// printf("nTocLen==============%d\n", nTocLen);
	for(i=0; i<nTocLen; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		tmp[0] = tocs[i];
		index = toc_get_index(tocs[i]);
		if (index > MAX_FRAME_TYPE)
		{
			printf("Bad amr toc, index=%i\n", index);
			break;
		}

		framesz = amr_frame_sizes[index];
		nRead = bs_read_bytes_ex(payload, &tmp[1], framesz);
		nSize = nsamples * 2;
		
		Decoder_Interface_Decode(amrnb_dec, tmp, (short*) output, 0);
		/* nRet = fwrite(output, (size_t)1, nSize, fp); */
		nRet = nSize;
		memcpy(pOutput, output, nSize);
		// printf("nRet == %d, nSize == %d\n", nRet, nSize);

	} // end of for
	bs_free(payload);

	return nRet;
}

int amrnb_encode_init(int nMode)
{
	amrnb_mode = nMode;
	amrnb_enc = Encoder_Interface_init(amrnb_dtx);
	return 0;
}

int amrnb_encode_uninit()
{
	Encoder_Interface_exit(amrnb_enc);
	amrnb_enc = NULL;
	return 0;
}

int amrnb_encode(char *pData, int nSize, FILE *fp)
{
	int nRet = 0;
	unsigned int unitary_buff_size = sizeof (int16_t) * NUM_SAMPLES;
	unsigned int buff_size = unitary_buff_size * amrnb_ptime / 20;
	uint8_t tmp[OUT_MAX_SIZE];
	int16_t samples[buff_size];
	uint8_t	tmp1[20*OUT_MAX_SIZE];
	bs_t	*payload = NULL;
	int		nCmr = 0xF;
	int		nFbit = 1, nFTbits = 0, nQbit = 0;
	int		nReserved = 0, nPadding = 0;
	int		nFrameData = 0, framesz = 0, nWrite = 0;
	int		offset = 0;

	uint8_t output[OUT_MAX_SIZE * buff_size / unitary_buff_size + 1];
	int 	nOutputSize = 0;

	while (nSize >= buff_size)
	{
		memset(output, 0, sizeof(output));
		memcpy((uint8_t*)samples, pData, buff_size);
		payload = bs_new(output, OUT_MAX_SIZE * buff_size / unitary_buff_size + 1);
		
		nFrameData = 0; nWrite = 0;
		for (offset = 0; offset < buff_size; offset += unitary_buff_size)
		{
			int ret = Encoder_Interface_Encode(amrnb_enc, amrnb_mode, &samples[offset / sizeof (int16_t)], tmp, amrnb_dtx);
			if (ret <= 0 || ret > 32)
			{
				printf("Encoder returned %i\n", ret);
				continue;
			}
			nFbit = tmp[0] >> 7;
			nFbit = (offset+buff_size >= unitary_buff_size) ? 0 : 1;
			nFTbits = tmp[0] >> 3 & 0x0F;
			if(nFTbits > MAX_FRAME_TYPE)
			{
				printf("%s, Bad amr toc, index=%i (MAX=%d)\n", __func__, nFTbits, MAX_FRAME_TYPE);
				break;
			}
			nQbit = tmp[0] >> 2 & 0x01;
			framesz = amr_frame_sizes[nFTbits];
			
			// Frame 데이터를 임시로 복사
			memcpy(&tmp1[nFrameData], &tmp[1], framesz);
			nFrameData += framesz;
			
			// write TOC
			bs_write_u(payload, 1, nFbit);
			bs_write_u(payload, 4, nFTbits);
			bs_write_u(payload, 1, nQbit);
			if(b_octet_align == 1)
			{	// octet-align, add padding bit
				bs_write_u(payload, 2, nPadding);
			}

		} // end of for
		if(offset > 0)
		{
			nWrite = bs_write_bytes_ex(payload, tmp1, nFrameData);
		}
		
		nOutputSize = 1 + framesz;
		nRet = fwrite(output, (size_t)1, nOutputSize, fp);
	
		bs_free(payload);
		nSize -= buff_size;

	} // end of while
	return nRet;
}

// same as amrnb_encode
int amrnb_encode_buf(char *pData, int nSize, /*FILE *fp*/ char *pOutput)
{
	int nRet = 0;
	unsigned int unitary_buff_size = sizeof (int16_t) * NUM_SAMPLES;
	unsigned int buff_size = unitary_buff_size * amrnb_ptime / 20;
	uint8_t tmp[OUT_MAX_SIZE];
	int16_t samples[buff_size];
	uint8_t	tmp1[20*OUT_MAX_SIZE];
	bs_t	*payload = NULL;
	int		nCmr = 0xF;
	int		nFbit = 1, nFTbits = 0, nQbit = 0;
	int		nReserved = 0, nPadding = 0;
	int		nFrameData = 0, framesz = 0, nWrite = 0;
	int		offset = 0;

	uint8_t output[OUT_MAX_SIZE * buff_size / unitary_buff_size + 1];
	int 	nOutputSize = 0;

	while (nSize >= buff_size)
	{
		memset(output, 0, sizeof(output));
		memcpy((uint8_t*)samples, pData, buff_size);
		payload = bs_new(output, OUT_MAX_SIZE * buff_size / unitary_buff_size + 1);
		
		nFrameData = 0; nWrite = 0;
		for (offset = 0; offset < buff_size; offset += unitary_buff_size)
		{
			int ret = Encoder_Interface_Encode(amrnb_enc, amrnb_mode, &samples[offset / sizeof (int16_t)], tmp, amrnb_dtx);
			if (ret <= 0 || ret > 32)
			{
				printf("Encoder returned %i\n", ret);
				continue;
			}
			nFbit = tmp[0] >> 7;
			nFbit = (offset+buff_size >= unitary_buff_size) ? 0 : 1;
			nFTbits = tmp[0] >> 3 & 0x0F;
			if(nFTbits > MAX_FRAME_TYPE)
			{
				printf("%s, Bad amr toc, index=%i (MAX=%d)\n", __func__, nFTbits, MAX_FRAME_TYPE);
				break;
			}
			nQbit = tmp[0] >> 2 & 0x01;
			framesz = amr_frame_sizes[nFTbits];
			
			// Frame 데이터를 임시로 복사
			memcpy(&tmp1[nFrameData], &tmp[1], framesz);
			nFrameData += framesz;
			
			// write TOC
			bs_write_u(payload, 1, nFbit);
			bs_write_u(payload, 4, nFTbits);
			bs_write_u(payload, 1, nQbit);
			if(b_octet_align == 1)
			{	// octet-align, add padding bit
				bs_write_u(payload, 2, nPadding);
			}

		} // end of for
		if(offset > 0)
		{
			nWrite = bs_write_bytes_ex(payload, tmp1, nFrameData);
		}
		
		nOutputSize = 1 + framesz;
		/* nRet = fwrite(output, (size_t)1, nOutputSize, fp); */
		nRet = nOutputSize;
		memcpy(pOutput, output, nOutputSize);
		// printf("nRet == %d, nSize == %d\n", nRet, nOutputSize);
	
		bs_free(payload);
		nSize -= buff_size;

	} // end of while
	return nRet;
}
