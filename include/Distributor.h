
#ifndef DISTRIBUTOR_H
#define DISTRIBUTOR_H

#include "Wire.h"
#include <vector>
#include <algorithm>

template <typename DType, size_t channels>
class Distributor {
public:
    Distributor():
        _channels{channels},
        _cond_var{std::make_shared<CondVar>()}
        {}
    ~Distributor() {}

    void connect(WirePtr<DType, channels> output_wire) {
        _pending_outputs.push_back(output_wire->getInPtr());
    }

    size_t pushAudio(const DType* input_buffer, size_t input_samples, size_t channel_index = 0) {
        addPending();
        size_t sus_max = 0;
        for (InJackWeakPtr<DType, channels> output : _outputs) {
            InJackPtr<DType, channels> ptr = output.lock();
            if (!ptr)
                continue;
            size_t sus = ptr->pushAudio(input_buffer, input_samples, channel_index);
            sus_max = std::max(sus, sus_max);
        }
        return sus_max;
    }

    std::vector<size_t> pushAudio(const std::vector<DType*>* input_buffers, size_t input_samples) {
        addPending();
        std::vector<size_t> sus_max (_channels, 0);
        for (InJackWeakPtr<DType, channels> output : _outputs) {
            InJackPtr<DType, channels> ptr = output.lock();
            if (!ptr)
                continue;
            std::vector<size_t> sus = ptr->pushAudio(input_buffers, input_samples);
            std::transform(sus_max.begin(), sus_max.end(), sus.begin(), sus_max.begin(), 
                [](const size_t& a , const size_t& b){return std::max(a, b);});
        }
        return sus_max;
    }

    size_t getConnectedWireCount() const {
        return _outputs.size();
    }
    size_t getPendingWireCount() const {
        return _pending_outputs.size();
    }

    void signal() {
        _cond_var->signal();
    }

    std::shared_ptr<CondVar> getCondVar() {
        return _cond_var;
    }

    bool canRead() {
        bool ret = false;
        for (InJackWeakPtr<DType, channels> output : _outputs) {
            InJackPtr<DType, channels> ptr = output.lock();
            if (!ptr)
                continue;
            ret = ret | ptr->canRead();
        }
        return ret;
    }
private:
    void addPending() {
        for (InJackWeakPtr<DType, channels> pending_output : _pending_outputs) {
            _outputs.push_back(pending_output);
        }
        _pending_outputs.clear();
    }
    std::vector<InJackWeakPtr<DType, channels>> _outputs;
    std::vector<InJackWeakPtr<DType, channels>> _pending_outputs;
    size_t _channels;

    std::shared_ptr<CondVar> _cond_var;
};
#endif