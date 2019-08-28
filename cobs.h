/**
  *@file cobs.h
  *@brief header for Consistent Overhead Byte Stuffing
  *@author Jason Berger
  *@date 03/26/2019
  *
  *@info https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
  */


/**
  *@brief returns the maximum length of encoded data
  *@param len the length of the unencoded data
  *@return maximum length of data once it is encoded
  */
#define COBS_MAX_LEN(x) ((x) + (((x)+255)/254) + 1)


  typedef enum{
    COBS_OK=0,
    COBS_ERROR = -1
  }cobs_status_e;

/**
  *@brief Performs Byte stuffing and stores encoded data
  *@param ptr ptr to data to encode
  *@param len length of unencoded data
  *@param dst ptr to store endcoded data
  *@pre dst must have enough memory allocated for the encoded data, use COBS_MAX_LEN()
  *@return length of encoded data
  */
int cobs_encode(const uint8_t* ptr, int len, uint8_t* dst);

/**
  *@brief Decodes data by reversing the byte stuffing
  *@param ptr ptr to data to decode
  *@param len length of encoded data
  *@param dst ptr to store decoded data
  *@return length of decoded data
  */
int cobs_decode(const uint8_t* ptr, int len, uint8_t* dst);
