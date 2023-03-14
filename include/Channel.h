#ifndef CHANNEL_H
#define CHANNEL_H

#include "Wire.h"
#include "Aggregator.h"
#include "Distributor.h"

/*
The base class for a processing unit/channel
Connect units with wires. In-wires should have the same datatype and chanels. So do out-wires.
In/Out-wires can have different DType but should have the same channels.

Use _aggregator.popAudio(...) / _distributor.pushAudio(...) to read from upstream units and push
processed samples to downstream units.
*/
template <typename DTypeIn, typename DTypeOut, size_t channels>
class Channel {
public:
    Channel() : 
        _aggregator{},
        _distributor{} {

    }
    virtual ~Channel() {

    }

    /*
    Connect a wire to the aggregator, in each update cycle we
        1. read from the aggregator
        2. process read samples
        3. push to the distributor
    */
    void connectIn(WirePtr<DTypeIn, channels> input_wire) {
        _aggregator.connect(input_wire);
    }

    /*
    Connect a wire to the distributor, in each update cycle we
        1. read from the aggregator
        2. process read samples
        3. push to the distributor
    */
    void connectOut(WirePtr<DTypeOut, channels> output_wire) {
        _distributor.connect(output_wire);
    }

    /*
    Connect aggregator to upstream's distributor.
    The conditional variable is fired when there's new sample in the upstream.
    Can only listen to one distributor
    Example:
        _aggregator.wait()
        _aggregator.popAudio(...) to read new samples
    */
    void listenTo(std::shared_ptr<CondVar> other) {
        _aggregator.setCondVar(other);
    }

    /*
    Return distributor's conditional variable. Used in pair with listenTo
    Example:
        downstream.listenTo(upstream.getSignal())
        downstream's aggregator is signaled whenever upstream pushed new samples to its distributor.
    */
    std::shared_ptr<CondVar> getSignal() {
        return _distributor.getCondVar();
    }

protected: 
    Aggregator<DTypeIn, channels> _aggregator; // has a shared_ptr points to the condition var of upstream's _distributor
    Distributor<DTypeOut, channels> _distributor; // owns a conditional var. share it with downstream.
};

#endif