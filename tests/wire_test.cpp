#include <gtest/gtest.h>
#include <Wire.h>
#include <vector>

using namespace std;

class WireTest : public ::testing::Test {
    protected:
    void SetUp() override {
        // A wire with two channels
        in = wire.getInPtr();
        out = wire.getOutPtr();

        sus = 0;
        sus_vec.clear();
        outL = vector<int>(256, 0);
        outR = vector<int>(256, 0);
        outLR = vector<int*> {outL.data(), outR.data()};
    }

    void TearDown() override {}

    // The wire itself
    Wire<int, 2> wire{2, 5};
    std::weak_ptr<InJack<int, 2>> in;
    std::weak_ptr<OutJack<int, 2>> out;

    // Returned #samples
    size_t sus;
    vector<size_t> sus_vec;

    // Returned buffer
    vector<int> outL;
    vector<int> outR;
    vector<int*> outLR;
};

TEST_F(WireTest, InitialStates) {
    // Two channels
    EXPECT_EQ(wire.getChannels(), 2);
    EXPECT_EQ(out.lock()->getChannels(), 2);
    EXPECT_EQ(in.lock()->getChannels(), 2);

    // Spaces
    EXPECT_EQ(out.lock()->getSpace(0), 9);
    EXPECT_EQ(in.lock()->getSpace(0), 9);
    EXPECT_EQ(out.lock()->getSpace(1), 9);
    EXPECT_EQ(in.lock()->getSpace(1), 9);

    // Stored
    EXPECT_EQ(out.lock()->getStored(0), 0);
    EXPECT_EQ(in.lock()->getStored(0), 0);
    EXPECT_EQ(out.lock()->getStored(1), 0);
    EXPECT_EQ(in.lock()->getStored(1), 0);
}

TEST_F(WireTest, PushAudioVector) {
    vector<int> inL;
    vector<int> inR;
    vector<int*> inLR;
    std::vector<std::vector<int>> status{};

    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();


    // 1,2,3,4,_,_,_,_,_,_ 
    // 5,6,7,8,_,_,_,_,_,_ 
    inL = vector<int>{1,2,3,4};
    inR = vector<int>{5,6,7,8};
    inLR = vector<int*>{inL.data(), inR.data()};

    sus_vec = inPtr->pushAudio(&inLR, 4);

    EXPECT_EQ(outPtr->getStored(0), 4);
    EXPECT_EQ(outPtr->getStored(1), 4);
    EXPECT_EQ(outPtr->getSpace(0), 5);
    EXPECT_EQ(outPtr->getSpace(1), 5);
    EXPECT_EQ(sus_vec[0], 4);
    EXPECT_EQ(sus_vec[1], 4);

    status = outPtr->status();
    std::vector<std::vector<int>> ans = {
        {1,2,3,4,0,0,0,0,0,0},
        {5,6,7,8,0,0,0,0,0,0}};
    EXPECT_EQ(status, ans);
    
    // Push to fill the buffer
    // 1,2,3,4, 5,6,7,8,9,_
    // 5,6,7,8, 9,1,2,3,4,_
    inL = vector<int>{5,6,7,8,9,777};
    inR = vector<int>{9,1,2,3,4,777};
    inLR = vector<int*>{inL.data(), inR.data()};

    sus_vec = inPtr->pushAudio(&inLR, 6);

    EXPECT_EQ(outPtr->getStored(0), 9);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 0);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus_vec[0], 5);
    EXPECT_EQ(sus_vec[1], 5);

    status = outPtr->status();
    ans = {
        {1,2,3,4,5,6,7,8,9,0},
        {5,6,7,8,9,1,2,3,4,0}};
    EXPECT_EQ(status, ans);

    // Pop three elements out
    // (1,2,3),4,5,6,7,8,9,_
    // (5,6,7),8,9,1,2,3,4,_
    int pholder[10] {0};
    outPtr->popAudio(pholder, 3, 0);
    outPtr->popAudio(pholder, 3, 1);

    // Push and round back to head
    // 7,(2,3),4,5,6,7,8,9,8
    // 2,(6,7),8,9,1,2,3,4,3
    inL = vector<int>{8,7};
    inR = vector<int>{3,2};
    inLR = vector<int*>{inL.data(), inR.data()};

    sus_vec = inPtr->pushAudio(&inLR, 2);

    EXPECT_EQ(outPtr->getStored(0), 8);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 1);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus_vec[0], 2);
    EXPECT_EQ(sus_vec[1], 2);

    status = outPtr->status();
    ans = {
        {7,2,3,4,5,6,7,8,9,8},
        {2,6,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans);

    // Start from the round-back write position, push to fill the buffer again
    // 7,6,(3),4,5,6,7,8,9,8
    // 2,1,(7),8,9,1,2,3,4,3
    inL = vector<int>{6,666,666,666};
    inR = vector<int>{1,111,111,111};
    inLR = vector<int*>{inL.data(), inR.data()};

    sus_vec = inPtr->pushAudio(&inLR, 4);

    EXPECT_EQ(outPtr->getStored(0), 9);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 0);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus_vec[0], 1);
    EXPECT_EQ(sus_vec[1], 1);

    status = outPtr->status();
    ans = {
        {7,6,3,4,5,6,7,8,9,8},
        {2,1,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans);

    // Can't push anymore
    // 7,6,(3),4,5,6,7,8,9,8
    // 2,1,(7),8,9,1,2,3,4,3
    inL = vector<int>{404,404,404};
    inR = vector<int>{404,404,404};
    inLR = vector<int*>{inL.data(), inR.data()};

    sus_vec = inPtr->pushAudio(&inLR, 3);

    EXPECT_EQ(outPtr->getStored(0), 9);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 0);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus_vec[0], 0);
    EXPECT_EQ(sus_vec[1], 0);

    status = outPtr->status();
    ans = {
        {7,6,3,4,5,6,7,8,9,8},
        {2,1,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans); // MAX
}

TEST_F(WireTest, PushAudioPtr) {
    vector<int> inR;
    std::vector<std::vector<int>> status{};

    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();


    // _,_,_,_,_,_,_,_,_,_ 
    // 5,6,7,8,_,_,_,_,_,_ 
    inR = vector<int>{5,6,7,8};

    sus = inPtr->pushAudio(inR.data(), 4, 1);

    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 4);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 5);
    EXPECT_EQ(sus, 4);

    status = outPtr->status();
    std::vector<std::vector<int>> ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {5,6,7,8,0,0,0,0,0,0}};
    EXPECT_EQ(status, ans);
    
    // Push to fill the buffer
    // _,_,_,_,_,_,_,_,_,_ 
    // 5,6,7,8, 9,1,2,3,4,_
    inR = vector<int>{9,1,2,3,4,777};

    sus = inPtr->pushAudio(inR.data(), 6, 1);

    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus, 5);

    status = outPtr->status();
    ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {5,6,7,8,9,1,2,3,4,0}};
    EXPECT_EQ(status, ans);

    // Pop three elements out
    // _,_,_,_,_,_,_,_,_,_ 
    // (5,6,7),8,9,1,2,3,4,_
    int pholder[10] {0};
    outPtr->popAudio(pholder, 3, 0);
    outPtr->popAudio(pholder, 3, 1);

    // Push and round back to head
    // _,_,_,_,_,_,_,_,_,_ 
    // 2,(6,7),8,9,1,2,3,4,3
    inR = vector<int>{3,2};

    sus = inPtr->pushAudio(inR.data(), 2, 1);

    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus, 2);

    status = outPtr->status();
    ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {2,6,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans);

    // Start from the round-back write position, push to fill the buffer again
    // _,_,_,_,_,_,_,_,_,_ 
    // 2,1,(7),8,9,1,2,3,4,3
    inR = vector<int>{1,111,111,111};

    sus = inPtr->pushAudio(inR.data(), 4, 1);

    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus, 1);

    status = outPtr->status();
    ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {2,1,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans);

    // Can't push anymore
    // _,_,_,_,_,_,_,_,_,_ 
    // 2,1,(7),8,9,1,2,3,4,3
    inR = vector<int>{404,404,404};

    sus = inPtr->pushAudio(inR.data(), 3, 1);

    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 9);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 0);
    EXPECT_EQ(sus, 0);

    status = outPtr->status();
    ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {2,1,7,8,9,1,2,3,4,3}};
    EXPECT_EQ(status, ans); // MAX
}

TEST_F(WireTest, PopAudioVector) {
    vector<int> inL {1,2,3,4,5,6,7,8,9,8};
    vector<int> inR {8,9,8,7,6,5,4,3,2,1};
    vector<int*> inLR {inL.data(), inR.data()};

    std::vector<std::vector<int>> status{};

    // Fill with inital values
    // 1,2,3,4,5,6,7,8,9,0
    // 8,9,8,7,6,5,4,3,2,0
    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();
    sus_vec = inPtr->pushAudio(&inLR, 10);
    EXPECT_EQ(sus_vec[0], 9);
    EXPECT_EQ(sus_vec[1], 9);
    std::vector<std::vector<int>> ans = {
        {1,2,3,4,5,6,7,8,9,0},
        {8,9,8,7,6,5,4,3,2,0}};
    status = outPtr->status();
    EXPECT_EQ(status, ans);

    // Read 
    // -1,-2,-3,-4,5,6,7,8,9,0
    // -8,-9,-8,-7,6,5,4,3,2,0
    sus_vec = outPtr->popAudio(&outLR, 4);
    EXPECT_EQ(outPtr->getStored(0), 5);
    EXPECT_EQ(outPtr->getStored(1), 5);
    EXPECT_EQ(outPtr->getSpace(0), 4);
    EXPECT_EQ(outPtr->getSpace(1), 4);
    EXPECT_EQ(sus_vec[0], 4);
    EXPECT_EQ(sus_vec[1], 4);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {1,2,3,4},
        {8,9,8,7}};
    EXPECT_EQ(status, ans);

    // Read till the end
    // -1,-2,-3,-4,-5,-6,-7,-8,-9,0
    // -8,-9,-8,-7,-6,-5,-4,-3,-2,0
    sus_vec = outPtr->popAudio(&outLR, 13);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 5);
    EXPECT_EQ(sus_vec[1], 5);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {5,6,7,8,9},
        {6,5,4,3,2}};
    EXPECT_EQ(status, ans);

    // Fill with values again
    // 2,3,4,5,6,7,8,9,-9,1
    // 9,8,7,6,5,4,3,2,-2,8
    sus_vec = inPtr->pushAudio(&inLR, 10);
    EXPECT_EQ(sus_vec[0], 9);
    EXPECT_EQ(sus_vec[1], 9);
    ans = {
        {2,3,4,5,6,7,8,9,9,1},
        {9,8,7,6,5,4,3,2,2,8}};
    status = outPtr->status();
    EXPECT_EQ(status, ans);

    // Read and round back
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus_vec = outPtr->popAudio(&outLR, 1);
    EXPECT_EQ(outPtr->getStored(0), 8);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 1);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus_vec[0], 1);
    EXPECT_EQ(sus_vec[1], 1);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {1},
        {8}};
    EXPECT_EQ(status, ans);

    // Read till the end again
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus_vec = outPtr->popAudio(&outLR, 8);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 8);
    EXPECT_EQ(sus_vec[1], 8);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {2,3,4,5,6,7,8,9},
        {9,8,7,6,5,4,3,2}};
    EXPECT_EQ(status, ans);

    // Read from empty array
    sus_vec = outPtr->popAudio(&outLR, 8);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 0);
    EXPECT_EQ(sus_vec[1], 0);
}

TEST_F(WireTest, PopAudioPtr) {
    vector<int> inL {1,2,3,4,5,6,7,8,9,8};
    vector<int> inR {8,9,8,7,6,5,4,3,2,1};
    vector<int*> inLR {inL.data(), inR.data()};

    std::vector<int> status{};
    std::vector<std::vector<int>> _status{};

    // Fill with inital values
    // 0,0,0,0,0,0,0,0,0,0
    // 8,9,8,7,6,5,4,3,2,0
    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();
    sus = inPtr->pushAudio(inR.data(), 10, 1);
    EXPECT_EQ(sus, 9);
    std::vector<std::vector<int>> _ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {8,9,8,7,6,5,4,3,2,0}};
    _status = outPtr->status();
    EXPECT_EQ(_status, _ans);

    // Read 
    // -1,-2,-3,-4,5,6,7,8,9,0
    // -8,-9,-8,-7,6,5,4,3,2,0
    sus = outPtr->popAudio(outR.data(), 4, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 5);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 4);
    EXPECT_EQ(sus, 4);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    std::vector<int> ans = std::vector<int>{8,9,8,7};
    EXPECT_EQ(status, ans);

    // Read till the end
    // -1,-2,-3,-4,-5,-6,-7,-8,-9,0
    // -8,-9,-8,-7,-6,-5,-4,-3,-2,0
    sus = outPtr->popAudio(outR.data(), 13, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 5);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{6,5,4,3,2};
    EXPECT_EQ(status, ans);

    // Fill with values again
    // 2,3,4,5,6,7,8,9,-9,1
    // 9,8,7,6,5,4,3,2,-2,8
    sus = inPtr->pushAudio(inR.data(), 10, 1);
    EXPECT_EQ(sus, 9);
    _ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {9,8,7,6,5,4,3,2,2,8}};
    _status = outPtr->status();
    EXPECT_EQ(_status, _ans);

    // Read and round back
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus = outPtr->popAudio(outR.data(), 1, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus, 1);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{8};
    EXPECT_EQ(status, ans);

    // Read till the end again
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus = outPtr->popAudio(outR.data(), 8, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 8);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{9,8,7,6,5,4,3,2};
    EXPECT_EQ(status, ans);

    // Read from empty array
    sus = outPtr->popAudio(outR.data(), 8, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 0);
}

TEST_F(WireTest, PopAndSumAudioPtr) {
    vector<int> inL {1,2,3,4,5,6,7,8,9,8};
    vector<int> inR {8,9,8,7,6,5,4,3,2,1};
    vector<int*> inLR {inL.data(), inR.data()};

    std::vector<std::vector<int>> status{};

    vector<int> initial {2,4,5,1,2,4,7,9,8,3};
    std::copy_n(initial.begin(), 10, outL.begin());
    std::copy_n(initial.begin(), 10, outR.begin());

    // Fill with inital values
    // 1,2,3,4,5,6,7,8,9,0
    // 8,9,8,7,6,5,4,3,2,0
    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();
    sus_vec = inPtr->pushAudio(&inLR, 10);
    EXPECT_EQ(sus_vec[0], 9);
    EXPECT_EQ(sus_vec[1], 9);
    std::vector<std::vector<int>> ans = {
        {1,2,3,4,5,6,7,8,9,0},
        {8,9,8,7,6,5,4,3,2,0}};
    status = outPtr->status();
    EXPECT_EQ(status, ans);

    // Read 
    // -1,-2,-3,-4,5,6,7,8,9,0
    // -8,-9,-8,-7,6,5,4,3,2,0
    sus_vec = outPtr->popAndSumAudio(&outLR, 4);
    EXPECT_EQ(outPtr->getStored(0), 5);
    EXPECT_EQ(outPtr->getStored(1), 5);
    EXPECT_EQ(outPtr->getSpace(0), 4);
    EXPECT_EQ(outPtr->getSpace(1), 4);
    EXPECT_EQ(sus_vec[0], 4);
    EXPECT_EQ(sus_vec[1], 4);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {3,6,8,5},
        {10,13,13,8}};
    EXPECT_EQ(status, ans);

    // Read till the end
    // -1,-2,-3,-4,-5,-6,-7,-8,-9,0
    // -8,-9,-8,-7,-6,-5,-4,-3,-2,0
    sus_vec = outPtr->popAndSumAudio(&outLR, 13);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 5);
    EXPECT_EQ(sus_vec[1], 5);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {8,12,15,13,11},
        {16,18,17,11,4}};
    EXPECT_EQ(status, ans);

    // Fill with values again
    // 2,3,4,5,6,7,8,9,-9,1
    // 9,8,7,6,5,4,3,2,-2,8
    sus_vec = inPtr->pushAudio(&inLR, 10);
    EXPECT_EQ(sus_vec[0], 9);
    EXPECT_EQ(sus_vec[1], 9);
    ans = {
        {2,3,4,5,6,7,8,9,9,1},
        {9,8,7,6,5,4,3,2,2,8}};
    status = outPtr->status();
    EXPECT_EQ(status, ans);

    // Read and round back
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus_vec = outPtr->popAndSumAudio(&outLR, 1);
    EXPECT_EQ(outPtr->getStored(0), 8);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 1);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus_vec[0], 1);
    EXPECT_EQ(sus_vec[1], 1);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {9},
        {24}};
    EXPECT_EQ(status, ans);

    // Read till the end again
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus_vec = outPtr->popAndSumAudio(&outLR, 8);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 8);
    EXPECT_EQ(sus_vec[1], 8);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    ans = {
        {11,15,19,18,17,11,15,18},
        {33,26,24,17,9,8,10,11}};
    EXPECT_EQ(status, ans);

    // Read from empty array
    sus_vec = outPtr->popAndSumAudio(&outLR, 8);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus_vec[0], 0);
    EXPECT_EQ(sus_vec[1], 0);
}

TEST_F(WireTest, PopAndSumAudioVector) {
    vector<int> inL {1,2,3,4,5,6,7,8,9,8};
    vector<int> inR {8,9,8,7,6,5,4,3,2,1};
    vector<int*> inLR {inL.data(), inR.data()};

    std::vector<int> status{};
    std::vector<std::vector<int>> _status{};

    vector<int> initial {2,4,5,1,2,4,7,9,8,3};
    std::copy_n(initial.begin(), 10, outR.begin());

    // Fill with inital values
    // 0,0,0,0,0,0,0,0,0,0
    // 8,9,8,7,6,5,4,3,2,0
    std::shared_ptr<InJack<int, 2>> inPtr = in.lock();
    std::shared_ptr<OutJack<int, 2>> outPtr= out.lock();
    sus = inPtr->pushAudio(inR.data(), 10, 1);
    EXPECT_EQ(sus, 9);
    std::vector<std::vector<int>> _ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {8,9,8,7,6,5,4,3,2,0}};
    _status = outPtr->status();
    EXPECT_EQ(_status, _ans);

    // Read 
    // -1,-2,-3,-4,5,6,7,8,9,0
    // -8,-9,-8,-7,6,5,4,3,2,0
    sus = outPtr->popAndSumAudio(outR.data(), 4, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 5);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 4);
    EXPECT_EQ(sus, 4);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    std::vector<int> ans = std::vector<int>{10,13,13,8};
    EXPECT_EQ(status, ans);

    // Read till the end
    // -1,-2,-3,-4,-5,-6,-7,-8,-9,0
    // -8,-9,-8,-7,-6,-5,-4,-3,-2,0
    sus = outPtr->popAndSumAudio(outR.data(), 13, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 5);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{16,18,17,11,4};
    EXPECT_EQ(status, ans);

    // Fill with values again
    // 2,3,4,5,6,7,8,9,-9,1
    // 9,8,7,6,5,4,3,2,-2,8
    sus = inPtr->pushAudio(inR.data(), 10, 1);
    EXPECT_EQ(sus, 9);
    _ans = {
        {0,0,0,0,0,0,0,0,0,0},
        {9,8,7,6,5,4,3,2,2,8}};
    _status = outPtr->status();
    EXPECT_EQ(_status, _ans);

    // Read and round back
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus = outPtr->popAndSumAudio(outR.data(), 1, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 8);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 1);
    EXPECT_EQ(sus, 1);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{24};
    EXPECT_EQ(status, ans);

    // Read till the end again
    // 2,3,4,5,6,7,8,9,-9,-1
    // 9,8,7,6,5,4,3,2,-2,-8
    sus = outPtr->popAndSumAudio(outR.data(), 8, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 8);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    ans = std::vector<int>{33,26,24,17,9,8,10,11};
    EXPECT_EQ(status, ans);

    // Read from empty array
    sus = outPtr->popAndSumAudio(outR.data(), 8, 1);
    EXPECT_EQ(outPtr->getStored(0), 0);
    EXPECT_EQ(outPtr->getStored(1), 0);
    EXPECT_EQ(outPtr->getSpace(0), 9);
    EXPECT_EQ(outPtr->getSpace(1), 9);
    EXPECT_EQ(sus, 0);
}