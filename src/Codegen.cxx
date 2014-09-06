//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include "Codegen.h"
#include "AudioBufferInput.h"
#include "Fingerprint.h"
#include "Whitening.h"
#include "SubbandAnalysis.h"
#include "Fingerprint.h"
#include "Common.h"
#include "Base64.h"

#include <stdio.h>
#include <stdlib.h>


using std::string;
using std::vector;

extern "C" {

    const char *fingerprint_js_float(float *samples, int numSamples) {
        Codegen *c = new Codegen(samples, numSamples, 20);
        std::string codestring = c->getCodeString();
        return codestring.c_str();
    }
    const char *fingerprint_js_short(short *input, int numSamples) {
        float *samples = (float*)malloc(sizeof(float) * numSamples);
        long sampleCount = 0;
        for (long i = 0; i < numSamples; i++) {
            float data = (float) input[i] / 32768.0f;
            samples[sampleCount++] = data;
        }
        return fingerprint_js_float(samples, numSamples);
    }

const char *fingerprint_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    // Samples are 2 bytes each, so we need size/2 floats to put the samples in
    size = size / 2;
    short *samples = (short*)malloc(sizeof(short) * size);
    long sampleCount = 0;
    for (long i = 0; i < size; i++) {
        short v;
        fread(&v, 2, 1, fp);
        samples[sampleCount++] = v;
    }

    return fingerprint_js_short(samples, size);
}
}

Codegen::Codegen(const float* pcm, unsigned int numSamples, int start_offset) {

    Whitening *pWhitening = new Whitening(pcm, numSamples);
    pWhitening->Compute();

    AudioBufferInput *pAudio = new AudioBufferInput();
    pAudio->SetBuffer(pWhitening->getWhitenedSamples(), pWhitening->getNumSamples());

    SubbandAnalysis *pSubbandAnalysis = new SubbandAnalysis(pAudio);
    pSubbandAnalysis->Compute();

    Fingerprint *pFingerprint = new Fingerprint(pSubbandAnalysis, start_offset);
    pFingerprint->Compute();

    _CodeString = createCodeString(pFingerprint->getCodes());
    _NumCodes = pFingerprint->getCodes().size();

    delete pFingerprint;
    delete pSubbandAnalysis;
    delete pWhitening;
    delete pAudio;
}

string Codegen::createCodeString(vector<FPCode> vCodes) {
    if (vCodes.size() < 3) {
        return "";
    }
    std::ostringstream codestream;
    codestream << std::setfill('0') << std::hex;
    for (uint i = 0; i < vCodes.size(); i++)
        codestream << std::setw(5) << vCodes[i].frame;

    for (uint i = 0; i < vCodes.size(); i++) {
        int hash = vCodes[i].code;
        codestream << std::setw(5) << hash;
    }
    return compress(codestream.str());
}

string Codegen::compress(const string& s) {
    long max_compressed_length = s.size()*2;
    unsigned char *compressed = new unsigned char[max_compressed_length];

    // zlib the code string
    z_stream stream;
    stream.next_in = (Bytef*)(unsigned char*)s.c_str();
    stream.avail_in = (uInt)s.size();
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    do {
        stream.next_out = compressed;
        stream.avail_out = max_compressed_length;
        if(deflate(&stream, Z_FINISH) == Z_STREAM_END) break;
    } while (stream.avail_out == 0);
    uint compressed_length = stream.total_out;
    deflateEnd(&stream);

    // base64 the zlib'd code string
    string encoded = base64_encode(compressed, compressed_length, true);
    delete [] compressed;
    return encoded;
}

