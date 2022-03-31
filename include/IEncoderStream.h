#ifndef IENCODERSTREAM_H
#define IENCODERSTREAM_H

#include "IOParams.h"

class IEncoderStream{
public:
    virtual ~IEncoderStream(){}
    virtual void encodeSumIntoBuffer(EncoderInputData*, EncoderOutputData*) = 0;
};

#endif
