/* 
 * h264bitstream - a library for reading and writing H.264 video
 * Copyright (C) 2005-2007 Auroras Entertainment, LLC
 * Copyright (C) 2008-2011 Avail-TVN
 * 
 * Written by Alex Izvorski <aizvorski@gmail.com> and Alex Giladi <alex.giladi@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _H264_BS_H
#define _H264_BS_H        1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint8_t* start;
	uint8_t* p;
	uint8_t* end;
	int bits_left;
} bs_t;

#define _OPTIMIZE_BS_ 1

#if ( _OPTIMIZE_BS_ > 0 )
#ifndef FAST_U8
#define FAST_U8
#endif
#endif


static bs_t* bs_new(uint8_t* buf, size_t size);
static void bs_free(bs_t* b);
static bs_t* bs_clone( bs_t* dest, const bs_t* src );
static bs_t*  bs_init(bs_t* b, uint8_t* buf, size_t size);
static uint32_t bs_byte_aligned(bs_t* b);
static int bs_eof(bs_t* b);
static int bs_overrun(bs_t* b);
static int bs_pos(bs_t* b);

static uint32_t bs_peek_u1(bs_t* b);
static uint32_t bs_read_u1(bs_t* b);
static uint32_t bs_read_u(bs_t* b, int n);
static uint32_t bs_read_f(bs_t* b, int n);
static uint32_t bs_read_u8(bs_t* b);
static uint32_t bs_read_ue(bs_t* b);
static int32_t  bs_read_se(bs_t* b);

static void bs_write_u1(bs_t* b, uint32_t v);
static void bs_write_u(bs_t* b, int n, uint32_t v);
static void bs_write_f(bs_t* b, int n, uint32_t v);
static void bs_write_u8(bs_t* b, uint32_t v);
static void bs_write_ue(bs_t* b, uint32_t v);
static void bs_write_se(bs_t* b, int32_t v);

static int bs_read_bytes(bs_t* b, uint8_t* buf, int len);
static int bs_write_bytes(bs_t* b, uint8_t* buf, int len);
static int bs_skip_bytes(bs_t* b, int len);
static uint32_t bs_next_bits(bs_t* b, int nbits);
// IMPLEMENTATION

static inline bs_t* bs_init(bs_t* b, uint8_t* buf, size_t size)
{
    b->start = buf;
    b->p = buf;
    b->end = buf + size;
    b->bits_left = 8;
    return b;
}

static inline bs_t* bs_new(uint8_t* buf, size_t size)
{
    bs_t* b = (bs_t*)malloc(sizeof(bs_t));
    bs_init(b, buf, size);
    return b;
}

static inline void bs_free(bs_t* b)
{
    free(b);
}

static inline bs_t* bs_clone(bs_t* dest, const bs_t* src)
{
    dest->start = src->p;
    dest->p = src->p;
    dest->end = src->end;
    dest->bits_left = src->bits_left;
    return dest;
}

static inline uint32_t bs_byte_aligned(bs_t* b) 
{ 
    return (b->bits_left == 8);
}

static inline int bs_eof(bs_t* b) { if (b->p >= b->end) { return 1; } else { return 0; } }

static inline int bs_overrun(bs_t* b) { if (b->p > b->end) { return 1; } else { return 0; } }

static inline int bs_pos(bs_t* b) { if (b->p > b->end) { return (b->end - b->start); } else { return (b->p - b->start); } }

static inline int bs_bytes_left(bs_t* b) { return (b->end - b->p); }

static inline uint32_t bs_read_u1(bs_t* b)
{
    uint32_t r = 0;
    
    b->bits_left--;

    if (! bs_eof(b))
    {
        r = ((*(b->p)) >> b->bits_left) & 0x01;
    }

    if (b->bits_left == 0) { b->p ++; b->bits_left = 8; }

    return r;
}

static inline void bs_skip_u1(bs_t* b)
{    
    b->bits_left--;
    if (b->bits_left == 0) { b->p ++; b->bits_left = 8; }
}

static inline uint32_t bs_peek_u1(bs_t* b)
{
    uint32_t r = 0;

    if (! bs_eof(b))
    {
        r = ((*(b->p)) >> ( b->bits_left - 1 )) & 0x01;
    }
    return r;
}


static inline uint32_t bs_read_u(bs_t* b, int n)
{
    uint32_t r = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        r |= ( bs_read_u1(b) << ( n - i - 1 ) );
    }
    return r;
}

static inline void bs_skip_u(bs_t* b, int n)
{
    int i;
    for ( i = 0; i < n; i++ ) 
    {
        bs_skip_u1( b );
    }
}

static inline uint32_t bs_read_f(bs_t* b, int n) { return bs_read_u(b, n); }

static inline uint32_t bs_read_u8(bs_t* b)
{
#ifdef FAST_U8
    if (b->bits_left == 8 && ! bs_eof(b)) // can do fast read
    {
        uint32_t r = b->p[0];
        b->p++;
        return r;
    }
#endif
    return bs_read_u(b, 8);
}

static inline uint32_t bs_read_ue(bs_t* b)
{
    int32_t r = 0;
    int i = 0;

    while( (bs_read_u1(b) == 0) && (i < 32) && (!bs_eof(b)) )
    {
        i++;
    }
    r = bs_read_u(b, i);
    r += (1 << i) - 1;
    return r;
}

static inline int32_t bs_read_se(bs_t* b) 
{
    int32_t r = bs_read_ue(b);
    if (r & 0x01)
    {
        r = (r+1)/2;
    }
    else
    {
        r = -(r/2);
    }
    return r;
}


static inline void bs_write_u1(bs_t* b, uint32_t v)
{
    b->bits_left--;

    if (! bs_eof(b))
    {
        // FIXME this is slow, but we must clear bit first
        // is it better to memset(0) the whole buffer during bs_init() instead? 
        // if we don't do either, we introduce pretty nasty bugs
        (*(b->p)) &= ~(0x01 << b->bits_left);
        (*(b->p)) |= ((v & 0x01) << b->bits_left);
    }

    if (b->bits_left == 0) { b->p ++; b->bits_left = 8; }
}

static inline void bs_write_u(bs_t* b, int n, uint32_t v)
{
    int i;
    for (i = 0; i < n; i++)
    {
        bs_write_u1(b, (v >> ( n - i - 1 ))&0x01 );
    }
}

static inline void bs_write_f(bs_t* b, int n, uint32_t v) { bs_write_u(b, n, v); }

static inline void bs_write_u8(bs_t* b, uint32_t v)
{
#ifdef FAST_U8
    if (b->bits_left == 8 && ! bs_eof(b)) // can do fast write
    {
        b->p[0] = v;
        b->p++;
        return;
    }
#endif
    bs_write_u(b, 8, v);
}

static inline void bs_write_ue(bs_t* b, uint32_t v)
{
    static const int len_table[256] =
    {
        1,
        1,
        2,2,
        3,3,3,3,
        4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    };

    int len;

    if (v == 0)
    {
        bs_write_u1(b, 1);
    }
    else
    {
        v++;

        if (v >= 0x01000000)
        {
            len = 24 + len_table[ v >> 24 ];
        }
        else if(v >= 0x00010000)
        {
            len = 16 + len_table[ v >> 16 ];
        }
        else if(v >= 0x00000100)
        {
            len =  8 + len_table[ v >>  8 ];
        }
        else 
        {
            len = len_table[ v ];
        }

        bs_write_u(b, 2*len-1, v);
    }
}

static inline void bs_write_se(bs_t* b, int32_t v)
{
    if (v <= 0)
    {
        bs_write_ue(b, -v*2);
    }
    else
    {
        bs_write_ue(b, v*2 - 1);
    }
}

static inline int bs_read_bytes(bs_t* b, uint8_t* buf, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len) { actual_len = b->end - b->p; }
    if (actual_len < 0) { actual_len = 0; }
    memcpy(buf, b->p, actual_len);
    if (len < 0) { len = 0; }
    b->p += len;
    return actual_len;
}

static inline int bs_read_bytes_ex(bs_t* b, uint8_t* buf, int len)
{
    int i=0, mask = 0xFF, shift = 0;
    int value = 0, remain = 0;

    if(b->bits_left == 8)
    {
        return bs_read_bytes(b, buf, len);
    }
    
    for(i=0; i<len; i++)
    {
        if(bs_eof(b)) break;

        remain = b->bits_left;      // 6비트가 남아 있다.
        shift = (8 - b->bits_left); // 6비트를 처리하므로, 나머지는 2 bits 이다.

        // Get value from b (1st)
        mask = 0xFF >> shift;       // 6비트를 추출하려면 2bits만 이동한다.
        //printf("[%02d] *buf[0x%02x] v[0x%02x] m[0x%02x] 1..b[0x%02x]\n", i, *buf, value, mask, *(b->p));
        
        value = *(b->p) & mask;
        *buf |= (value << shift);   // 6비트를 설정했으므로, 2bits를 이동해야 한다.
        //printf("[%02d] *buf[0x%02x] v[0x%02x] m[0x%02x] 2..b[0x%02x]\n", i, *buf, value, mask, *(b->p));

        b->p ++; b->bits_left = 8;  // Next bit
        if(bs_eof(b))
        {   // 더 이상 읽을 데이터가 없으므로 여기서 마무리 한다.
            i++;    // buf에 상위 비트를 설정했으므로, 길이를 증가시킨 후 종료한다.
            break;
        }

        // Get value from b (2nd)
        mask = 0xFF >> remain;      // 2bits 를 추출하려면 6 bits 이동한다.
        value = *(b->p) >> (remain);// 2bits 를 추츨하려면 6 bits 이동한다.
        b->bits_left = remain;      // 2 bits를 추출하므로, 6 bits 남아 있음.
        *buf |= value & mask;

        //printf("[%02d] *buf[0x%02x] v[0x%02x] m[0x%02x] 3..b[0x%02x]\n", i, *buf, value, mask, *(b->p));
        buf++;
    } // end of for
    return i;
}

static inline int bs_write_bytes(bs_t* b, uint8_t* buf, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len) { actual_len = b->end - b->p; }
    if (actual_len < 0) { actual_len = 0; }
    memcpy(b->p, buf, actual_len);
    if (len < 0) { len = 0; }
    b->p += len;
    return actual_len;
}

static inline int bs_write_bytes_ex(bs_t* b, uint8_t* buf, int len)
{
    int i=0, mask = 0xFF, shift = 0;
    int value = 0, remain = 0;

    if(b->bits_left == 8)
    {
        return bs_write_bytes(b, buf, len);
    }
    
    for(i=0; i<len; i++)
    {
        if ( bs_eof(b)) break;

        mask = 0xFF >> b->bits_left;
        shift = (8-b->bits_left);
        // Get value from buffer
        value = *buf >> shift;
        remain = *buf & mask;
        //printf("[%02d] *buf[0x%02x] v[0x%02x] r[0x%02x] 1..b[0x%02x]\n", i, *buf, value, remain, *(b->p));

        // Write value (1st)
        (*(b->p)) |= (value);
        //printf("[%02d] *buf[0x%02x] v[0x%02x] r[0x%02x] 2..b[0x%02x]\n", i, *buf, value, remain, *(b->p));
        b->p ++; b->bits_left = 8;

        // Write remain (2nd)
        b->bits_left = 8-shift;
        (*(b->p)) = 0x00;
        (*(b->p)) |= (remain << (b->bits_left));
        //printf("[%02d] *buf[0x%02x] v[0x%02x] r[0x%02x] 3..b[0x%02x]\n", i, *buf, value, remain, *(b->p));
        
        buf++;
    } // end of for
    return i;
}

static inline int bs_skip_bytes(bs_t* b, int len)
{
    int actual_len = len;
    if (b->end - b->p < actual_len) { actual_len = b->end - b->p; }
    if (actual_len < 0) { actual_len = 0; }
    if (len < 0) { len = 0; }
    b->p += len;
    return actual_len;
}

static inline uint32_t bs_next_bits(bs_t* bs, int nbits)
{
   bs_t b;
   bs_clone(&b,bs);
   return bs_read_u(&b, nbits);
}

static inline uint64_t bs_next_bytes(bs_t* bs, int nbytes)
{
   int i = 0;
   uint64_t val = 0;

   if ( (nbytes > 8) || (nbytes < 1) ) { return 0; }
   if (bs->p + nbytes > bs->end) { return 0; }

   for ( i = 0; i < nbytes; i++ ) { val = ( val << 8 ) | bs->p[i]; }
   return val;
}

#define bs_print_state(b) fprintf( stderr,  "%s:%d@%s: b->p=0x%02hhX, b->left = %d\n", __FILE__, __LINE__, __FUNCTION__, *b->p, b->bits_left )

#ifdef __cplusplus
}
#endif

#endif
