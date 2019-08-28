
#ifdef UNIT_TESTING_ENABLED



extern "C"{
#include "cobs.c"
#include "cobs_fifo.c"
}


#include <gtest/gtest.h>


#define FRAME_0_LEN 21
#define FRAME_1_LEN 12
uint8_t cobs_Frame0[FRAME_0_LEN] = {0x00, 0x2D, 0x41, 0x00, 0x02, 0x03, 0x43, 0x00, 0x0C, 0x00, 0x2D, 0x20, 0x0C, 0x59, 0x59, 0x5A, 0x00, 0x0C, 0x00, 0x0C, 0x00};
uint8_t cobs_Frame1[FRAME_1_LEN] = {0x20, 0x0C, 0x59, 0x59, 0x5A, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x3D, 0x62};

uint8_t cobs_Encoded0[] = {0x01, 0x03, 0x2D, 0x41, 0x04, 0x02, 0x03, 0x43, 0x02, 0x0C, 0x07, 0x2D, 0x20, 0x0C, 0x59, 0x59, 0x5A, 0x02, 0x0C, 0x02, 0x0C,0x01, 0x00};

uint8_t cobs_Comp0[FRAME_0_LEN];
uint8_t cobs_Comp1[FRAME_1_LEN];

int cobs_encodedLen;
int cobs_decodedLen;
uint8_t cobs_tmp0[64];
uint8_t cobs_tmp1[64];

void cobs_show_buf(uint8_t* buf, int len)
{
  for(int i =0; i < len; i++)
  {
    printf("0x%02X, ",  buf[i]);
    //ASSERT_EQ(cobs_Comp0[i],cobs_Frame0[i]);
  }
  printf("\n");
}

TEST(Cobs_Test, max)
{
    int max = COBS_MAX_LEN(FRAME_0_LEN);

    ASSERT_EQ(max, FRAME_0_LEN+2);
}

TEST(Cobs_Test, encode )
{

  cobs_encodedLen = cobs_encode(cobs_Frame0, FRAME_0_LEN, cobs_tmp0);
  ASSERT_EQ(FRAME_0_LEN+2, cobs_encodedLen);

  for(int i =0; i < cobs_encodedLen; i++)
  {
    ASSERT_EQ(cobs_tmp0[i],cobs_Encoded0[i]);
  }

}

TEST(Cobs_Test, decode)
{
  cobs_decodedLen = cobs_decode(cobs_tmp0,cobs_encodedLen,cobs_tmp1);
  ASSERT_EQ(FRAME_0_LEN, cobs_decodedLen);
  for(int i =0; i < FRAME_0_LEN; i++)
  {
    ASSERT_EQ(cobs_tmp1[i],cobs_Frame0[i]);
  }

}


TEST(CobsFifo_Test, pushframe )
{
  cobs_fifo_t cobsFifo;
  int nextLen;
  cobs_fifo_init(&cobsFifo,128);

  cobs_fifo_push_frame(&cobsFifo, cobs_Frame0, FRAME_0_LEN);  //encoding adds one byte

  ASSERT_EQ(cobsFifo.mCount, FRAME_0_LEN + 2); //1 byte of Overhead and one delimiter
  ASSERT_EQ(cobsFifo.mNextLen,FRAME_0_LEN +2); //location of delimiter



  cobs_fifo_deinit(&cobsFifo);
}

TEST(CobsFifo_Test, popframe )
{
  cobs_fifo_t cobsFifo;
  int nextLen;
  cobs_fifo_init(&cobsFifo,128);

  cobs_fifo_push_frame(&cobsFifo, cobs_Frame0, FRAME_0_LEN);  //encoding adds one byte
  cobs_fifo_pop_frame(&cobsFifo, cobs_Comp0, FRAME_0_LEN);

  for(int i =0; i < FRAME_0_LEN; i++)
  {
    ASSERT_EQ(cobs_Comp0[i],cobs_Frame0[i]);
  }

  ASSERT_EQ(cobsFifo.mCount, 0);
  ASSERT_EQ(cobsFifo.mFrameCount, 0);
  ASSERT_EQ(cobsFifo.mNextLen, 0);

  cobs_fifo_deinit(&cobsFifo);
}



TEST(CobsFifo_Test, multiframes )
{
  cobs_fifo_t cobsFifo;
  int nextLen;
  int decodedLen;
  cobs_fifo_init(&cobsFifo,128);

  cobs_fifo_push_frame(&cobsFifo, cobs_Frame0, FRAME_0_LEN);  //encoding adds one byte
  cobs_fifo_push_frame(&cobsFifo, cobs_Frame1, FRAME_1_LEN);

  nextLen = cobs_fifo_get_next_len(&cobsFifo);
  ASSERT_EQ(nextLen,FRAME_0_LEN +2 );
  ASSERT_EQ(cobsFifo.mCount, FRAME_0_LEN + FRAME_1_LEN + 4); // 2 ohb + 2 delimiters
  ASSERT_EQ(cobsFifo.mNextLen,FRAME_0_LEN+2);

  decodedLen = cobs_fifo_pop_frame(&cobsFifo, cobs_Comp0, FRAME_0_LEN);
  ASSERT_EQ(decodedLen,FRAME_0_LEN);

  for(int i =0; i < FRAME_0_LEN; i++)
  {
    ASSERT_EQ(cobs_Comp0[i],cobs_Frame0[i]);
  }

  ASSERT_EQ(cobsFifo.mCount, FRAME_1_LEN+2);
  ASSERT_EQ(cobsFifo.mNextLen,FRAME_1_LEN+2);

  for(int i =0; i < FRAME_1_LEN; i++)
  {
    ASSERT_EQ(cobs_Comp0[i],cobs_Frame0[i]);
  }

  decodedLen = cobs_fifo_pop_frame(&cobsFifo, cobs_Comp1, FRAME_1_LEN);
  ASSERT_EQ(decodedLen,FRAME_1_LEN);

  ASSERT_EQ(cobsFifo.mCount, 0);
  ASSERT_EQ(cobsFifo.mNextLen,0);

  cobs_fifo_deinit(&cobsFifo);
}





#endif
