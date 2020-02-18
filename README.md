# COBS
Module for Consistent Overhead Byte Stuffing
https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing


Consistent Overhead Byte Stuffing (COBS) is an algorithm for encoding data bytes that results in efficient, reliable, unambiguous packet framing regardless of packet content, thus making it easy for receiving applications to recover from malformed packets. It employs a particular byte value, typically zero, to serve as a packet delimiter (a special value that indicates the boundary between packets). When zero is used as a delimiter, the algorithm replaces each zero data byte with a non-zero value so that no zero data bytes will appear in the packet and thus be misinterpreted as packet boundaries.

## cobs.c/cobs.h

These provide the basic cobs utility for encoding/decoding a buffer of data.


## cobs_fifo.c / cobs_fifo.h

This is a fifo which uses cobs encoding to keep track of 'frames' inside of the fifo. a frame is a single buffer of data.

<br>

#### Working with frames
You can push/pop entire frames with the fifo
```c
cobs_fifo_t fifo;
uint8_t buf[32];            //tmp buffer
int len;

cobs_fio_init(&fifo, 256); // create a cobs fifo that can store 256 bytes 

uint8_t frameA[] = { 0x11, 0x22, 0x00, 0x33};
uint8_t frameB[] = { 0x12, 0x34};

cobs_fifo_push_frame(&fifo, frameA, 4); //push frame A into fifo. The frame is encoded as it is pushed into the fifo
//fifo->mNextLen is now 6. because frame A has 4 bytes + overhead byte and 1 byte for the delimiter

cobs_fifo_push_frame(&fifo, frameB, 2); //push frame B into fifo. The frame is encoded as it is pushed into the fifo
//fifo->mNextLen is still 6. because frame A is still the first frame in the buffer

len = cobs_pop_frame(&fifo, buf, 32); //pop and decode next frame from fifo
//fifo->mNextLen is now 4 because the next frame is frame B (2 bytes + 1 overhead + 1 delimiter)

len = cobs_pop_frame(&fifo, &buf[len], 32); //pop and decode next frame from fifo
//len will be the size of frame B decoded (2 bytes), buf = [0x12, 0x34]
 ```

<br>

#### Working with Raw Bytes
if you are sending or receiving bytes over serial and need to handle encoded data, you can use the _buf functions instead of _frame

**Sender**
```c
uint8_t frameA[] = { 0x11, 0x22, 0x00, 0x33};
uint8_t buf[256];

cobs_fifo_push_frame(&fifo, frameA, 4); //push frame A into fifo. The frame is encoded as it is pushed into the fifo


int len = cobs_fifo_pop_buf(&fifo, buf, 32 ); // pop the encoded frame for sending over serial 
//len is 6 (4 data bytes + 1 overhead + 1 delimiter )

//write delimited data to serial
uart_tx(buf, len); // buf will be [ 0x03, 0x11, 0x22, 0x02, 0x33, 0x00]

```

<br>

**Receiver**
```c
uint8_t buf[256];

int len = uart_rx(buf, 256); // using buf from send example [ 0x03, 0x11, 0x22, 0x02, 0x33, 0x00]

cobs_fifo_push_buf(&fifo, buf, len); //push raw data into fifo 


len = cobs_fifo_pop_frame(&fifo, buf, 32 ); // pop and decode next frame
//len = 4, buf = [ 0x11, 0x22, 0x00, 0x33 ]
```