#ifndef CODEC_711_H
#define CODEC_711_H

#define G711_DATA_LENGTH 160

void G711encode(short* inbuf, unsigned char* outbuf);

void G711decode(unsigned char* inbuf, short* outbuf);

#endif