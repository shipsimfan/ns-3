#ifndef CODEC_726_H
#define CODEC_726_H

#define G726_DATA_LENGTH 80

void G726encode(short* inbuf, unsigned char* outbuf);

void G726decode(unsigned char* inbuf, short* outbuf);

#endif