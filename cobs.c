/**
  *@file cobs.c
  *@brief source for Consistent Overhead Byte Stuffing
  *@author Jason Berger
  *@date 03/26/2019
  *
  *@info https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
  */

#include "cobs.h"

int cobs_encode(const uint8_t* ptr, int len, uint8_t* dst)
{
  const uint8_t *start = dst, *end = ptr + len;
	uint8_t code, *code_ptr; /* Where to insert the leading count */

  code_ptr = dst++ , code = 1;
	while (ptr < end)
  {
		if (code != 0xFF)
    {
			uint8_t c = *ptr++;
			if (c != 0)
      {
				*dst++ = c;
				code++;
				continue;
			}
		}
    *code_ptr = code;
    code_ptr = dst++ , code = 1;

	}
  *code_ptr++ = code;
  *dst++ = 0; //add delimiter

  return (int)(dst-start);
}

int cobs_decode(const uint8_t* ptr, int len, uint8_t* dst)
{
  const uint8_t *start = dst;
  const uint8_t *end = ptr + len;
	uint8_t code = 0xFF, copy = 0;

  for (; ptr < end; copy--)
  {
    if (copy != 0)
    {
      *dst++ = *ptr++;
    }
    else
    {
      if (code != 0xFF)
        *dst++ = 0;

      copy = code = *ptr++;

      if (code == 0)
        break; /* Source length too long */
    }
  }

  return (dst - start) -1; //we dont include the delim in the length return
}
