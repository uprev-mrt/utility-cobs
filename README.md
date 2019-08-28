# COBS
Module for Consistent Overhead Byte Stuffing
https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing


Consistent Overhead Byte Stuffing (COBS) is an algorithm for encoding data bytes that results in efficient, reliable, unambiguous packet framing regardless of packet content, thus making it easy for receiving applications to recover from malformed packets. It employs a particular byte value, typically zero, to serve as a packet delimiter (a special value that indicates the boundary between packets). When zero is used as a delimiter, the algorithm replaces each zero data byte with a non-zero value so that no zero data bytes will appear in the packet and thus be misinterpreted as packet boundaries.
