#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include "Wire.h"
#include "CondVar.h"
#include <vector>
#include <algorithm>
#include <memory>

template <typename DType, size_t channels>
class Aggregator {
public:
    Aggregator() :
        _channels{channels} ,
        _cond_var{}
        {}

    Aggregator(std::shared_ptr<CondVar> cond_var) :
        _channels{channels} ,
        _cond_var{cond_var}
        {}
    ~Aggregator() {}

    void connect(WirePtr<DType, channels> input_wire) {
        _pending_inputs.push_back(input_wire->getOutPtr());
        std::shared_ptr<CondVar> ptr = _cond_var.lock();
        if (ptr) {
            _pending_inputs.back()->addCondVar(ptr);
        }
    }

    size_t popAudio(DType* output_buffer, size_t output_samples, size_t channel_index = 0) {
        addPending();
        size_t sus_max = 0;
        for (OutJackPtr<DType, channels> input : _inputs) {
            size_t sus = input->popAndSumAudio(output_buffer, output_samples, channel_index);
            sus_max = std::max(sus, sus_max);
        }
        return sus_max;
    }

    std::vector<size_t> popAudio(std::vector<DType*>* output_buffers, size_t output_samples) {
        addPending();
        std::vector<size_t> sus_max (_channels, 0);
        for (OutJackPtr<DType, channels> input : _inputs) {
            std::vector<size_t> sus = input->popAndSumAudio(output_buffers, output_samples);
            std::transform(sus_max.begin(), sus_max.end(), sus.begin(), sus_max.begin(), 
                [](const size_t& a , const size_t& b){return std::max(a, b);});
        }
        return sus_max;
    }

    bool canRead() {
        bool ret = false;
        for (OutJackPtr<DType, channels> _input : _inputs) {
            ret = ret | _input->canRead();
        }
        return ret;
    }

    size_t getSamplesPerFrame() {
        //return _inputs[0]->getSamplesPerFrame();
        return 8192;
    }

    size_t getConnectedWireCount() const {
        return _inputs.size();
    }
    size_t getPendingWireCount() const {
        return _pending_inputs.size();
    }

    void wait() {
        std::shared_ptr<CondVar> ptr = _cond_var.lock();
        if (ptr) {
            ptr->wait();
        }
    }

    void setCondVar(std::shared_ptr<CondVar> other) {
        _cond_var = other;
    }
private:
    void addPending() {
        for (OutJackPtr<DType, channels> pending_input : _pending_inputs) {
            _inputs.push_back(pending_input);
        }
        _pending_inputs.clear();
    }
    std::vector<OutJackPtr<DType, channels>> _inputs;
    std::vector<OutJackPtr<DType, channels>> _pending_inputs;
    size_t _channels;

    // Points to the conditional variable in Distributor
    std::weak_ptr<CondVar> _cond_var;
};
#endif