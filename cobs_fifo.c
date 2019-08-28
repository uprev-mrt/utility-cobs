/**
  *@file cobs_fifo.c
  *@brief source for fifo that implements Consisten Overhead ByteStuffing
  *@author Jason Berger
  *@date 03/26/2019
  *
  *@info https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
  */

#include "cobs.h"
#include "cobs_fifo.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
  *@brief finds the next zero
  *@param fifo ptr to the fifo
  */
static int cobs_fifo_find_next_len(cobs_fifo_t* fifo);

void cobs_fifo_init(cobs_fifo_t* fifo, int len)
{
  fifo->mBuffer = (uint8_t*) malloc(len);
  fifo->mHead = 0;
  fifo->mTail = 0;
  fifo->mCount = 0;
  MRT_MUTEX_CREATE(fifo->mMutex);
  fifo->mFrameCount = 0;
  fifo->mNextLen = 0;
  fifo->mMaxLen = len;
}


void cobs_fifo_deinit(cobs_fifo_t* fifo)
{
  MRT_MUTEX_DELETE(fifo->mMutex);
  free(fifo->mBuffer);
}


cobs_fifo_status_e cobs_fifo_push(cobs_fifo_t* fifo, uint8_t data)
{
  MRT_MUTEX_LOCK(fifo->mMutex);

    // next is where head will point to after this write.
    int next = fifo->mHead + 1;

    if (next > fifo->mMaxLen)
    {
        next = 0;
    }
    if (next == fifo->mTail) // check if circular buffer is full
    {
		MRT_MUTEX_UNLOCK(fifo->mMutex);
        return COBS_FIFO_OVERFLOW;
    }

    fifo->mCount++;

    //With byte Stuffing the only zero bytes are frame boundaries
    if(data == 0 )
    {
      fifo->mFrameCount++;

      //if next frame is not set yet
      if(fifo->mNextLen == 0)
      {
        fifo->mNextLen = fifo->mCount;
      }
    }

    fifo->mBuffer[fifo->mHead] = data;

    fifo->mHead = next;            // head to next data offset.

    MRT_MUTEX_UNLOCK(fifo->mMutex);

    return COBS_FIFO_OK;  // return success to indicate successful push.
}


cobs_fifo_status_e cobs_fifo_pop(cobs_fifo_t* fifo, uint8_t* data)
{
  MRT_MUTEX_LOCK(fifo->mMutex);
    // if the head isn't ahead of the tail, we don't have any characters
    if (fifo->mHead == fifo->mTail) // check if circular buffer is empty
    {
		MRT_MUTEX_UNLOCK(fifo->mMutex);
        return COBS_FIFO_UNDERFLOW;          // and return with an error
    }

    // next is where tail will point to after this read.
    int next = fifo->mTail + 1;
    if (next > fifo->mMaxLen)
    {
        next = 0;
    }

    *data = fifo->mBuffer[fifo->mTail];

    if(*data == 0)
    {
      fifo->mFrameCount--;
      if(fifo->mNextLen >=0)
        fifo->mNextLen--;
    }

    fifo->mTail = next;             // tail to next data offset.

    if(fifo->mCount > 0)
    {
        fifo->mCount--;
    }
    MRT_MUTEX_UNLOCK(fifo->mMutex);

    return COBS_FIFO_OK;  // return success to indicate successful push.
}


cobs_fifo_status_e cobs_fifo_push_buf(cobs_fifo_t* fifo, const uint8_t* data, int len)
{
  cobs_fifo_status_e result = COBS_FIFO_OK;

  for(int i=0; i < len; i++)
  {
    result = cobs_fifo_push(fifo,data[i]);
  }
  return result;
}


cobs_fifo_status_e cobs_fifo_pop_buf(cobs_fifo_t* fifo, uint8_t* data, int len)
{
  cobs_fifo_status_e result= COBS_FIFO_OK;

  for(int i=0; i < len; i++)
  {
    result = cobs_fifo_pop(fifo, &data[i]);
  }
  return result;
}


cobs_fifo_status_e cobs_fifo_push_frame(cobs_fifo_t* fifo, const uint8_t* data, int len)
{
  int maxEncodedLen = len + ((len+255)/254);
  if((maxEncodedLen + fifo->mCount) > fifo->mMaxLen)
    return COBS_FIFO_OVERFLOW;

  uint8_t encoded[maxEncodedLen];

  int encodedLen = cobs_encode(data, len, encoded);

  cobs_fifo_push_buf(fifo,encoded,encodedLen);

  return COBS_FIFO_OK;
}


int cobs_fifo_pop_frame(cobs_fifo_t* fifo, uint8_t* data, int len)
{
  if(fifo->mFrameCount ==0)
    return 0;

  int encodedLen = fifo->mNextLen;
  int decodedLen;

  uint8_t encoded[encodedLen];

  cobs_fifo_pop_buf(fifo,encoded,encodedLen);

  decodedLen = cobs_decode(encoded, encodedLen, data);


  cobs_fifo_find_next_len(fifo);

	return decodedLen; //we dont include the delim in the length return
}

int cobs_fifo_find_next_len(cobs_fifo_t* fifo)
{
  MRT_MUTEX_LOCK(fifo->mMutex);
  int i;

  if(fifo->mFrameCount ==0)
  {
    MRT_MUTEX_UNLOCK(fifo->mMutex);
    fifo->mNextLen = 0;
    return 0;
  }

  int addr = fifo->mTail;

  for( i=1; i < fifo->mCount; i++)
  {
    if(fifo->mBuffer[addr++] == 0)
    {
      break;
    }

    if(addr == fifo->mMaxLen)
      addr=0;
  }

  fifo->mNextLen = i;

  MRT_MUTEX_UNLOCK(fifo->mMutex);
  return i;
}

int cobs_fifo_get_next_len(cobs_fifo_t* fifo)
{
  MRT_MUTEX_LOCK(fifo->mMutex);
  int retVal = fifo->mNextLen;
  MRT_MUTEX_UNLOCK(fifo->mMutex);
  return retVal;
}
