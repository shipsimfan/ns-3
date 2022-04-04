#include "Codec.h"

#include <iostream>

#include "G726State.h"
#include "cx_g726.h"

using namespace std;

const int G726_SAMPLES_PER_FRAME = 160;

class G726 : public Codec
{
public :
	virtual void encode(short *inbuf, unsigned char *outbuf, int &length);
	virtual void decode(unsigned char *inbuf, short *outbuf, int &length);

	virtual const CodecFormatList &getSupportedFormats(){
		return formatlist;
	}


	G726();
	~G726();

protected:
	int pack(unsigned char *buf, unsigned char *cw, unsigned char num_cw, int bps);
	int unpack(unsigned char *cw, unsigned char *buf, unsigned char num_cw, int bps);
	
private:
	CodecFormatList formatlist;

	virtual void destroy(){
		delete this;
	}
};

CODEC_FACTORY_ENTRY(G726, g726)

G726::G726(){

	/* G726-40 **********************************************/
	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-40-8K-Mono",
			"ITU G.726-40 ADPCM codec. Sun Microsystems public implementation.",
			107,0,100,
			AudioFormat(AudioFormat::SIGNED16,8000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-40-16K-Mono",
			"ITU G.726-40 ADPCM codec. Sun Microsystems public implementation.",
			107,0,100,
			AudioFormat(AudioFormat::SIGNED16,16000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-40-32K-Mono",
			"ITU G.726-40 ADPCM codec. Sun Microsystems public implementation.",
			107,0,100,
			AudioFormat(AudioFormat::SIGNED16,32000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-40-48K-Mono",
			"ITU G.726-40 ADPCM codec. Sun Microsystems public implementation.",
			107,0,100,
			AudioFormat(AudioFormat::SIGNED16,48000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	/* G726-32 ***********************************************/
	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-32-8K-Mono",
			"ITU G.726-32 ADPCM codec. Sun Microsystems public implementation.",
			2,0,80,
			AudioFormat(AudioFormat::SIGNED16,8000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-32-16K-Mono",
			"ITU G.726-32 ADPCM codec. Sun Microsystems public implementation.",
			104,0,80,
			AudioFormat(AudioFormat::SIGNED16,16000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-32-32K-Mono",
			"ITU G.726-32 ADPCM codec. Sun Microsystems public implementation.",
			105,0,80,
			AudioFormat(AudioFormat::SIGNED16,32000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-32-48K-Mono",
			"ITU G.726-32 ADPCM codec. Sun Microsystems public implementation.",
			106,0,80,
			AudioFormat(AudioFormat::SIGNED16,48000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	/* G726-24 ***********************************************/
	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-24-8K-Mono",
			"ITU G.726-24 ADPCM codec. Sun Microsystems public implementation.",
			2,0,60,
			AudioFormat(AudioFormat::SIGNED16,8000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-24-16K-Mono",
			"ITU G.726-24 ADPCM codec. Sun Microsystems public implementation.",
			104,0,60,
			AudioFormat(AudioFormat::SIGNED16,16000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-24-32K-Mono",
			"ITU G.726-24 ADPCM codec. Sun Microsystems public implementation.",
			105,0,60,
			AudioFormat(AudioFormat::SIGNED16,32000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-24-48K-Mono",
			"ITU G.726-24 ADPCM codec. Sun Microsystems public implementation.",
			106,0,60,
			AudioFormat(AudioFormat::SIGNED16,48000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	/* G726-16 ***********************************************/
	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-16-8K-Mono",
			"ITU G.726-16 ADPCM codec. Sun Microsystems public implementation.",
			96,0,40,
			AudioFormat(AudioFormat::SIGNED16,8000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-16-16K-Mono",
			"ITU G.726-16 ADPCM codec. Sun Microsystems public implementation.",
			97,0,40,
			AudioFormat(AudioFormat::SIGNED16,16000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-16-32K-Mono",
			"ITU G.726-16 ADPCM codec. Sun Microsystems public implementation.",
			98,0,40,
			AudioFormat(AudioFormat::SIGNED16,32000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	formatlist.insert(formatlist.end(),
		CodecFormat(
			"G726-16-48K-Mono",
			"ITU G.726-16 ADPCM codec. Sun Microsystems public implementation.",
			99,0,40,
			AudioFormat(AudioFormat::SIGNED16,48000,16,1,
				G726_SAMPLES_PER_FRAME * AudioFormat::BYTES_PER_SAMPLE
			)
		)
	);

	codecState = new G726State;
}

G726::~G726(){
	delete codecState;
}

int G726::pack(unsigned char *buf, unsigned char *cw, unsigned char num_cw, int bps)
{
	int i, bits = 0, x = 0;

	for (i = 0; i < num_cw; i++) {
		buf[x] |= cw[i] << bits;
		bits += bps;

		if (bits > 8) {
			bits &= 0x07;
			x++;
			buf[x] |= cw[i] >> (bps - bits);
		}
	}
	return (num_cw * bps / 8);
}

int G726::unpack(unsigned char *cw, unsigned char *buf, unsigned char num_cw, int bps)
{
	int i = 0, bits = 0, x = 0;
	unsigned char mask = 0;

	while (i < bps) {
		mask |= 1 << i;
		i++;
	}

	for(i = 0; i < num_cw; i++) {
		cw[i] = (buf[x] >> bits) & mask;
		bits += bps;

		if (bits > 8) {
			bits &= 0x07;
			x++;
			cw[i] |= buf[x] << (bps - bits);
			cw[i] &= mask;
		}
	}
	return (num_cw * bps / 8);
}

void G726::encode(short *inbuf, unsigned char *outbuf, int &length)
{
	register short *s;
    int i;
    unsigned char cw[8]; /* Maximum of 8 codewords in octet aligned packing */
	g726_state *state = (g726_state*)codecState->getState();
	
    s = inbuf;
    //g = (g726_t*)encoder_state;
	
    outbuf = new unsigned char[codecFormat->getFrameSize()];
    length = codecFormat->getFrameSize();
	
    memset(outbuf, 0, length);

	int rate = 0;
	string name = codecFormat->getName();

	int pos = name.find("G726-");
	if(pos != string::npos){
		string ratestring = name.substr(pos + 5,2);
		rate = atoi(ratestring.c_str());
	}
	
	switch(rate){
	case 16:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 4) {
			cw[0] = g726_16_encoder(s[i], AUDIO_ENCODING_LINEAR, state);
			cw[1] = g726_16_encoder(s[i + 1], AUDIO_ENCODING_LINEAR, state);
			cw[2] = g726_16_encoder(s[i + 2], AUDIO_ENCODING_LINEAR, state);
			cw[3] = g726_16_encoder(s[i + 3], AUDIO_ENCODING_LINEAR, state);
			outbuf += pack(outbuf, cw, 4, 2);
		}
		break;
	case 24:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 8) {
			cw[0] = g726_24_encoder(s[i], AUDIO_ENCODING_LINEAR, state);
			cw[1] = g726_24_encoder(s[i + 1], AUDIO_ENCODING_LINEAR, state);
			cw[2] = g726_24_encoder(s[i + 2], AUDIO_ENCODING_LINEAR, state);
			cw[3] = g726_24_encoder(s[i + 3], AUDIO_ENCODING_LINEAR, state);
			cw[4] = g726_24_encoder(s[i + 4], AUDIO_ENCODING_LINEAR, state);
			cw[5] = g726_24_encoder(s[i + 5], AUDIO_ENCODING_LINEAR, state);
			cw[6] = g726_24_encoder(s[i + 6], AUDIO_ENCODING_LINEAR, state);
			cw[7] = g726_24_encoder(s[i + 7], AUDIO_ENCODING_LINEAR, state);
			outbuf += pack(outbuf, cw, 8, 3);
		}
		break;
	case 32:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 2) {
			cw[0] = g726_32_encoder(s[i], AUDIO_ENCODING_LINEAR, state);
			cw[1] = g726_32_encoder(s[i + 1], AUDIO_ENCODING_LINEAR, state);
			outbuf += pack(outbuf, cw, 2, 4);
		}
		break;
	case 40:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 8) {
			cw[0] = g726_40_encoder(s[i], AUDIO_ENCODING_LINEAR, state);
			cw[1] = g726_40_encoder(s[i + 1], AUDIO_ENCODING_LINEAR, state);
			cw[2] = g726_40_encoder(s[i + 2], AUDIO_ENCODING_LINEAR, state);
			cw[3] = g726_40_encoder(s[i + 3], AUDIO_ENCODING_LINEAR, state);
			cw[4] = g726_40_encoder(s[i + 4], AUDIO_ENCODING_LINEAR, state);
			cw[5] = g726_40_encoder(s[i + 5], AUDIO_ENCODING_LINEAR, state);
			cw[6] = g726_40_encoder(s[i + 6], AUDIO_ENCODING_LINEAR, state);
			cw[7] = g726_40_encoder(s[i + 7], AUDIO_ENCODING_LINEAR, state);
			outbuf += pack(outbuf, cw, 8, 5);
		}
		break;
	default:
		break;
	}
}

void G726::decode(unsigned char *inbuf, short *outbuf, int &length)
{
	unsigned char cw[8];
	int i;
	
	g726_state *state = (g726_state*)codecState->getState(); 
	
	int rate = 0;
	string name = codecFormat->getName();

	outbuf = (short *)new unsigned char[codecFormat->getFrameSize()];
	length = codecFormat->getFrameSize();

	int pos = name.find("G726-");
	if(pos != string::npos){
		string ratestring = name.substr(pos + 5,2);
		rate = atoi(ratestring.c_str());
	}
	
	switch(rate) {
	case 16:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 4) {
			inbuf += unpack(cw, inbuf, 4, 2);
			outbuf[i + 0] = (short)g726_16_decoder(cw[0], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 1] = (short)g726_16_decoder(cw[1], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 2] = (short)g726_16_decoder(cw[2], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 3] = (short)g726_16_decoder(cw[3], AUDIO_ENCODING_LINEAR, state);
		}	
		break;	
	case 24:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 8) {
			inbuf += unpack(cw, inbuf, 8, 3);
			outbuf[i + 0] = (short)g726_24_decoder(cw[0], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 1] = (short)g726_24_decoder(cw[1], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 2] = (short)g726_24_decoder(cw[2], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 3] = (short)g726_24_decoder(cw[3], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 4] = (short)g726_24_decoder(cw[4], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 5] = (short)g726_24_decoder(cw[5], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 7] = (short)g726_24_decoder(cw[6], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 8] = (short)g726_24_decoder(cw[7], AUDIO_ENCODING_LINEAR, state);
		}
		break;
	case 32:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 2) {
			inbuf += unpack(cw, inbuf, 2, 4);
			outbuf[i + 0] = (short)g726_32_decoder(cw[0], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 1] = (short)g726_32_decoder(cw[1], AUDIO_ENCODING_LINEAR, state);
		}
		break;
	case 40:
		for(i = 0; i < G726_SAMPLES_PER_FRAME; i += 8) {
			inbuf += unpack(cw, inbuf, 8, 3);
			outbuf[i + 0] = (short)g726_40_decoder(cw[0], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 1] = (short)g726_40_decoder(cw[1], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 2] = (short)g726_40_decoder(cw[2], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 3] = (short)g726_40_decoder(cw[3], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 4] = (short)g726_40_decoder(cw[4], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 5] = (short)g726_40_decoder(cw[5], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 7] = (short)g726_40_decoder(cw[6], AUDIO_ENCODING_LINEAR, state);
			outbuf[i + 8] = (short)g726_40_decoder(cw[7], AUDIO_ENCODING_LINEAR, state);
		}
		break;
	}
}