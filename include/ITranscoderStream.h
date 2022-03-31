#ifndef ITRANSCODERSTREAM_H
#define ITRANSCODERSTREAM_H

#include "IOParams.h"

class ITranscoderStream{
public:
    virtual ~ITranscoderStream();
    virtual void transcodeToBuffer(TranscoderCallbackData*)
}

#endif