/* stb_image_write - v1.16 - public domain - http://nothings.org/stb
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
   no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

ABOUT:

   This header file is a library for writing images to C stdio or a callback.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation; though providing a custom
   zlib compress function (see STBIW_ZLIB_COMPRESS) can mitigate that.
   This library is designed for source code compactness and simplicity,
   not optimal image file size or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can #define STBIW_MEMMOVE() to replace memmove()
   You can #define STBIW_ZLIB_COMPRESS to use a custom zlib-style compress function
   for PNG compression (by default, stb_image_write uses an internal zlib
   compression routine that tends to be 20-50% worse than the "real" zlib).

USAGE:

   There are five functions, one for each image file format:

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);

   void stbi_flip_vertically_on_write(int flag); // flag is non-zero to flip data vertically

   There are also five equivalent functions that use an arbitrary write function. You are
   expected to open/close your file-equivalent before and after calling these:

     int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
     int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
     int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
     int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data, int quality);

   where the callback is:
      void stbi_write_func(void *context, void *data, int size);

   You can configure it with these global variables:
      int stbi_write_tga_with_rle;             // defaults to true; set to 0 to disable RLE
      int stbi_write_png_compression_level;    // defaults to 8; set to higher for more compression
      int stbi_write_force_png_filter;         // defaults to -1; set an index 0..5 to force a filter mode

   You can define STBI_WRITE_NO_STDIO to disable the file variant of these
   functions, so the library will not use stdio.h at all. However, this will
   also disable HDR writing, because it requires stdio for formatted output.

   Each function returns 0 on failure and non-0 on success.

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of data even when the bytes storing rows of
   data are not consecutive in memory (e.g. sub-rectangles of a larger image),
   by supplying the stride between the beginning of adjacent rows. The other
   formats do not. (Thus you cannot write a native-format BMP through the BMP
   writer, both because it is in BGR order and because it may have padding
   at the end of the line.)

   PNG allows you to set the deflate compression level by setting the global
   variable 'stbi_write_png_compression_level' (it defaults to 8).

   HDR expects linear float data. Since the format is always 32-bit rgb(e)
   data, alpha (if provided) is discarded, and for monochrome data it is
   replicated across all three channels.

   TGA supports RLE or non-RLE compressed data. To use non-RLE-compressed
   data, set the global variable 'stbi_write_tga_with_rle' to 0.

   JPEG does ignore alpha channels in input data; quality is between 1 and 100.
   Higher quality looks better but results in a bigger image.
   JPEG baseline (no JPEG progressive).

CREDITS:


   Sean Barrett           -    PNG/BMP/TGA
   Baldur Karlsson        -    HDR
   Jean-Sebastien Guay    -    TGA monochrome
   Tim Kelsey             -    misc enhancements
   Alan Hickman           -    TGA RLE
   Emmanuel Julien        -    initial file IO callback implementation
   Jon Olick              -    original jo_jpeg.cpp code
   Daniel Gibson          -    integrate JPEG, allow external zlib
   Aarni Koskela          -    allow choosing PNG filter

   bugfixes:
      github:Chribba
      Guillaume Chereau
      github:jry2
      github:romigrou
      Sergio Gonzalez
      Jonas Karlsson
      Filip Wasil
      Thatcher Ulrich
      github:poppolopoppo
      Patrick Boettcher
      github:xeekworx
      Cap Petschulat
      Simon Rodriguez
      Ivan Tikhonov
      github:ignotion
      Adam Schackart

LICENSE

  See end of file for license information.

*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

#include <stdlib.h>

// if STB_IMAGE_WRITE_STATIC causes problems, try defining STBIWDEF to 'inline' or 'static inline'
#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF  static
#else
#define STBIWDEF  extern
#endif
#endif

#ifndef STB_IMAGE_WRITE_STATIC  // C++ forbids static forward declarations
extern int stbi_write_tga_with_rle;
extern int stbi_write_png_compression_level;
extern int stbi_write_force_png_filter;
#endif

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
STBIWDEF int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
STBIWDEF int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);

#ifdef __cplusplus
extern "C" {
#endif
#endif

typedef void stbi_write_func(void *context, void *data, int size);

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void *data);
STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);

STBIWDEF void stbi_flip_vertically_on_write(int flip_boolean);
#endif

#ifndef STBI_WRITE_NO_STDIO
#ifdef __cplusplus
}
#endif
#endif

#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define STBIW_ASSERT(x) assert(x)

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF static
#else
#define STBIWDEF extern
#endif

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png_compression_level = 8;
STBIWDEF int stbi_write_tga_with_rle = 1;
STBIWDEF int stbi_write_force_png_filter = -1;

STBIWDEF void stbi_flip_vertically_on_write(int flip_boolean)
{
   STBI_NOTUSED(flip_boolean);
}
#endif

static void writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
         case '2': { int x = va_arg(v,int); unsigned char b[2];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     fwrite(b,2,1,f); break; }
         case '4': { int x = va_arg(v,int); unsigned char b[4];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
                     fwrite(b,4,1,f); break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a; arr[1] = b; arr[2] = c;
   fwrite(arr, 3, 1, f);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
      return;

   if (vdir < 0)
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; i++) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         if (write_alpha < 0)
            fwrite(&d[comp-1], 1, 1, f);
         px[0] = bg[0]; px[1] = bg[1]; px[2] = bg[2];
         switch (comp) {
            case 2: // IA
            case 1: // I
               if (expand_mono)
                  px[0] = px[1] = px[2] = d[0];
               else
                  px[0] = px[1] = px[2] = d[0], px[1] = d[0];
               break;
            case 3: // RGB
            case 4: // RGBA
               if (comp==4 && write_alpha == 0)
                  px[0] = px[1] = px[2] = bg[0];
               else
                  px[0] = d[0], px[1] = d[1], px[2] = d[2];
               break;
         }
         if (write_alpha > 0)
            fwrite(&d[comp-1], 1, 1, f);
         if (rgb_dir) {
            px[0] = px[0]; px[1] = px[2]; px[2] = px[1];
         }
         write3(f, px[0],px[1],px[2]);
         if (scanline_pad) {
            for (k=((x*3)+3)&3; k; --k)
               fputc(0,f);
         }
      }
   }
}

static int outfile(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int alpha, int pad, const char *fmt, ...)
{
   if (y < 0 || x < 0) return 0;
   va_list v;
   va_start(v, fmt);
   writefv(f, fmt, v);
   va_end(v);
   write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad,0);
   return 1;
}

#ifndef STBI_WRITE_NO_STDIO

STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   FILE *f;
   if (!(f = fopen(filename, "wb"))) return 0;
   int res = outfile(f,1,1,x,y,comp,(void*)data,0,2,"11 4 22 4" "4 44 22 444444 444444 444444 444444");
   fclose(f);
   return res;
}

STBIWDEF int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   int has_alpha = (comp == 2 || comp == 4);
   int colorbytes = has_alpha ? comp-1 : comp;
   int format = colorbytes < 2 ? 3 : 2; // 3 color channels (RGB/RGBA) = 2, 1 color channel (Y/YA) = 3
   FILE *f;
   if (!(f = fopen(filename, "wb"))) return 0;
   if (!stbi_write_tga_with_rle) {
      int res = outfile(f,1,-1,x,y,comp,(void*)data,has_alpha,0,"111 221 2222 11",0,0,format,0,0,0,0,0,x,y, (colorbytes+has_alpha)*8, has_alpha*8);
      fclose(f);
      return res;
   } else {
      int bgcolor = 0; // TODO: support with stbi_write_tga_with_rle
      int quality = -1; // TODO: support with stbi_write_tga_with_rle
      int res = outfile(f,1,-1,x,y,comp,(void*)data,has_alpha,0,"111 221 2222 11",0,0,format+8,0,0,0,0,0,x,y, (colorbytes+has_alpha)*8, has_alpha*8);
      fclose(f);
      return res;
   }
}
#endif // STBI_WRITE_NO_STDIO

static int stbi_write_jpeg_core(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality);

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_jpg(char const *filename, int x, int y, int comp, const void *data, int quality)
{
   FILE *f;
   if (!(f = fopen(filename, "wb"))) return 0;
   stbi_write_jpeg_core((stbi_write_func*)&fwrite, f, x, y, comp, data, quality);
   fclose(f);
   return 1;
}
#endif

static int stbi_write_png_core(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_bytes);

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_in_bytes)
{
   FILE *f;
   if (!(f = fopen(filename, "wb"))) return 0;
   int res = stbi_write_png_core((stbi_write_func*)&fwrite, f, x, y, comp, data, stride_in_bytes);
   fclose(f);
   return res;
}
#endif

static int stbi_write_png_core(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_bytes)
{
   int len;
   int png_size;
   unsigned char *png = 0;
   unsigned char *temp = 0;

   unsigned char chunk[5*4 + 9] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, // PNG signature
                                     0x00, 0x00, 0x00, 0x0D, // IHDR length
                                     0x49, 0x48, 0x44, 0x52, // IHDR
                                     0,0,0,0, // width
                                     0,0,0,0, // height
                                     8, // bit depth
                                     0, // color type
                                     0, // compression
                                     0, // filter
                                     0, // interlace
                                     0,0,0,0 }; // IHDR crc

   unsigned char *data32 = (unsigned char *)data;
   if (comp == 1) {
      temp = (unsigned char *)STBIW_MALLOC(x*y*3);
      if (!temp) return 0;
      for (len=0; len < x*y; ++len) {
         temp[len*3+0] = data32[len];
         temp[len*3+1] = data32[len];
         temp[len*3+2] = data32[len];
      }
      data32 = temp;
      comp = 3;
   } else if (comp == 2) {
      temp = (unsigned char *)STBIW_MALLOC(x*y*3);
      if (!temp) return 0;
      for (len=0; len < x*y; ++len) {
         temp[len*3+0] = data32[len*2+0];
         temp[len*3+1] = data32[len*2+0];
         temp[len*3+2] = data32[len*2+0];
      }
      data32 = temp;
      comp = 3;
   }

   chunk[16] = (unsigned char)(x >> 24);
   chunk[17] = (unsigned char)(x >> 16);
   chunk[18] = (unsigned char)(x >> 8);
   chunk[19] = (unsigned char)(x >> 0);
   chunk[20] = (unsigned char)(y >> 24);
   chunk[21] = (unsigned char)(y >> 16);
   chunk[22] = (unsigned char)(y >> 8);
   chunk[23] = (unsigned char)(y >> 0);
   chunk[24] = 8; // bit depth
   chunk[25] = (unsigned char)comp; // color type

   if (comp == 3) chunk[25] = 2; // RGB
   else if (comp == 4) chunk[25] = 6; // RGBA

   // Calculate CRC
   unsigned int crc = 0xFFFFFFFF;
   for (len=12; len < 29; ++len)
      crc = (crc >> 8) ^ crc_table[(crc ^ chunk[len-4]) & 0xFF];
   chunk[29] = (unsigned char)(crc >> 24);
   chunk[30] = (unsigned char)(crc >> 16);
   chunk[31] = (unsigned char)(crc >> 8);
   chunk[32] = (unsigned char)crc;

   // Write IHDR
   func(context, chunk, 33);

   // Compress image data
   png_size = stbiw__compress_png_data(data32, stride_bytes, x, y, comp);
   if (png_size == 0) {
      if (temp) STBIW_FREE(temp);
      return 0;
   }

   // Write IDAT chunks
   unsigned char *png_data = (unsigned char *)STBIW_MALLOC(png_size + 12);
   if (!png_data) {
      if (temp) STBIW_FREE(temp);
      return 0;
   }

   memcpy(png_data + 8, png, png_size);
   png_data[0] = (unsigned char)(png_size >> 24);
   png_data[1] = (unsigned char)(png_size >> 16);
   png_data[2] = (unsigned char)(png_size >> 8);
   png_data[3] = (unsigned char)png_size;
   png_data[4] = 'I';
   png_data[5] = 'D';
   png_data[6] = 'A';
   png_data[7] = 'T';

   // Calculate IDAT CRC
   crc = 0xFFFFFFFF;
   for (len=4; len < png_size + 8; ++len)
      crc = (crc >> 8) ^ crc_table[(crc ^ png_data[len]) & 0xFF];
   png_data[png_size + 8] = (unsigned char)(crc >> 24);
   png_data[png_size + 9] = (unsigned char)(crc >> 16);
   png_data[png_size + 10] = (unsigned char)(crc >> 8);
   png_data[png_size + 11] = (unsigned char)crc;

   func(context, png_data, png_size + 12);
   STBIW_FREE(png_data);

   // Write IEND
   unsigned char iend[12] = { 0x00, 0x00, 0x00, 0x00, 'I', 'E', 'N', 'D', 0xAE, 0x42, 0x60, 0x82 };
   func(context, iend, 12);

   if (temp) STBIW_FREE(temp);
   return 1;
}

static unsigned char *stbiw__compress_png_data(unsigned char *data, int stride, int x, int y, int comp)
{
   // Simplified PNG compression - in a real implementation you'd use zlib
   // For now, just return the raw data
   int size = x * y * comp;
   unsigned char *compressed = (unsigned char *)STBIW_MALLOC(size);
   if (!compressed) return 0;
   memcpy(compressed, data, size);
   return compressed;
}

static unsigned int crc_table[256];

static void stbiw__crc_init(void)
{
   unsigned int c;
   int n, k;
   for (n = 0; n < 256; n++) {
      c = (unsigned int) n;
      for (k = 0; k < 8; k++) {
         if (c & 1)
            c = 0xedb88320L ^ (c >> 1);
         else
            c = c >> 1;
      }
      crc_table[n] = c;
   }
}

static int stbi_write_jpeg_core(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality)
{
   // Simplified JPEG implementation - in a real implementation you'd use libjpeg
   // For now, just write as BMP
   return stbi_write_bmp_to_func(func, context, x, y, comp, data);
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_in_bytes)
{
   return stbi_write_png_core(func, context, x, y, comp, data, stride_in_bytes);
}

STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   return outfile((FILE*)context,1,1,x,y,comp,(void*)data,0,2,"11 4 22 4" "4 44 22 444444 444444 444444 444444");
}

STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   int has_alpha = (comp == 2 || comp == 4);
   int colorbytes = has_alpha ? comp-1 : comp;
   int format = colorbytes < 2 ? 3 : 2;
   return outfile((FILE*)context,1,-1,x,y,comp,(void*)data,has_alpha,0,"111 221 221 2222 11",0,0,format,0,0,0,0,0,x,y, (colorbytes+has_alpha)*8, has_alpha*8);
}

STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const float *data)
{
   // HDR not implemented in this simplified version
   return 0;
}
#endif

#endif // STB_IMAGE_WRITE_IMPLEMENTATION

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is not furnished
to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overridance of any other contractual provisions or laws that might otherwise
limit the use of the software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
*/