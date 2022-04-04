#include "codec-711.h"
#include <iostream>

using namespace std;

const int G711_SAMPLES_PER_FRAME = 160;
const int TABLE_SIZE = 8;
const int BIAS = 0x84; /* Bias for linear code. */
const int CLIP = 8159;
const int SIGN_BIT = 0x80;  /* Sign bit for a A-law byte. */
const int QUANT_MASK = 0xf; /* Quantization field mask. */
const int SEG_SHIFT = 4;    /* Left shift for segment number. */
const int SEG_MASK = 0x70;  /* Segment field mask. */

static short seg_uend[TABLE_SIZE] = {0x3F,  0x7F,  0xFF,  0x1FF,
                                     0x3FF, 0x7FF, 0xFFF, 0x1FFF};

// serach for value in table.
// returns location or TABLE_SIZE if not found
static short search(short val) {
    short i;

    for (i = 0; i < TABLE_SIZE; i++) {
        if (val <= seg_uend[i])
            return (i);
    }

    return (TABLE_SIZE);
}

unsigned char linear2ulaw(short pcm_val) {
    short mask;
    short seg;
    unsigned char uval;

    /* Get the sign and the magnitude of the value. */
    pcm_val = pcm_val >> 2;
    if (pcm_val < 0) {
        pcm_val = -pcm_val;
        mask = 0x7F;
    } else {
        mask = 0xFF;
    }
    if (pcm_val > CLIP)
        pcm_val = CLIP; /* clip the magnitude */
    pcm_val += (BIAS >> 2);

    /* Convert the scaled magnitude to segment number. */
    seg = search(pcm_val);

    /*
     * Combine the sign, segment, quantization bits;
     * and complement the code word.
     */
    if (seg >= 8) /* out of range, return maximum value. */
        return (unsigned char)(0x7F ^ mask);
    else {
        uval = (unsigned char)(seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
        return (uval ^ mask);
    }
}

short ulaw2linear(unsigned char u_val) {
    short t;

    /* Complement to obtain normal u-law value. */
    u_val = ~u_val;

    /*
     * Extract and bias the quantization bits. Then
     * shift up by the segment number and subtract out the bias.
     */
    t = ((u_val & QUANT_MASK) << 3) + BIAS;
    t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

    return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

void G711encode(short* inbuf, unsigned char* outbuf) {
    unsigned char *p, *pe;

    p = outbuf;
    pe = outbuf + G711_DATA_LENGTH;

    while (p != pe) {
        *p = linear2ulaw(*inbuf);
        p++;
        inbuf++;
    }
}

void G711decode(unsigned char* inbuf, short* outbuf) {
    unsigned char *p, *pe;

    p = inbuf;
    pe = inbuf + G711_DATA_LENGTH;

    while (p != pe) {
        *outbuf = ulaw2linear(*p);
        outbuf++;
        p++;
    }
}