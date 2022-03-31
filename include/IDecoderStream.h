#ifndef IDECODERSTREAM_H
#define IDECODERSTREAM_H

#include "IOParams.h"

class IDecoderStream{
public:
    virtual ~IDecoderStream();
    virtual void decoderSumToBuffer(DecoderInputData*, DecoderOutputData*) = 0;
}

#endif