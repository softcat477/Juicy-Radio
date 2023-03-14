#include <gtest/gtest.h>
#include <vector>

#include <Aggregator.h>
#include <Wire.h>

using namespace std;

class AggregatorTest : public ::testing::Test {
    protected:
    void SetUp() override {
        outL = vector<int>(256, 0);
        outR = vector<int>(256, 0);
        outLR = vector<int*> {outL.data(), outR.data()};

        wire1 = make_shared<Wire<int, 2>>(2, 5);
        wire2 = make_shared<Wire<int, 2>>(2, 5);
        wire3 = make_shared<Wire<int, 2>>(2, 5);
    }

    void TearDown() override {}

    // The wire itself
    WirePtr<int, 2> wire1;
    WirePtr<int, 2> wire2;
    WirePtr<int, 2> wire3;
    Aggregator<int, 2> agg;

    // Returned buffer
    vector<int> outL;
    vector<int> outR;
    vector<int*> outLR;
};

TEST_F(AggregatorTest, ConnectWire) {
    // Connect three wires to the pending list
    agg.connect(wire1);
    agg.connect(wire2);
    agg.connect(wire3);
    EXPECT_EQ(0, agg.getConnectedWireCount());
    EXPECT_EQ(3, agg.getPendingWireCount());

    // Pop while not sending samples to move pending wires to connected wires
    std::vector<size_t> sus_vec{};
    sus_vec = agg.popAudio(&outLR, 10);
    EXPECT_EQ(3, agg.getConnectedWireCount());
    EXPECT_EQ(sus_vec[0], 0);
    EXPECT_EQ(sus_vec[1], 0);
    EXPECT_EQ(3, agg.getConnectedWireCount());
    EXPECT_EQ(0, agg.getPendingWireCount());
}

TEST_F(AggregatorTest,AggregateVector) {
    // Connect two wires to the pending list
    agg.connect(wire1);
    agg.connect(wire2);
    EXPECT_EQ(0, agg.getConnectedWireCount());

    std::shared_ptr<InJack<int, 2>> in1 = wire1->getInPtr();
    std::shared_ptr<InJack<int, 2>> in2 = wire2->getInPtr();
    std::shared_ptr<InJack<int, 2>> in3 = wire3->getInPtr();
    std::vector<std::vector<int>> ans {};
    std::vector<std::vector<int>> status{};

    // Push to two wires
    std::vector<size_t> sus_vec{};
    std::vector<int> inL = vector<int>{1,2,3,4,5,6,7,8};
    std::vector<int> inR = vector<int>{5,6,7,8,1,2,3,4};
    std::vector<int*> inLR = vector<int*>{inL.data(), inR.data()};

    in1->pushAudio(&inLR, 4);
    in2->pushAudio(&inLR, 8);

    // Pop to sum two wires
    sus_vec = agg.popAudio(&outLR, 10);
    EXPECT_EQ(sus_vec[0], 8);
    EXPECT_EQ(sus_vec[1], 8);
    ans = std::vector<std::vector<int>>{
        {2,4,6,8,5,6,7,8},
        {10,12,14,16,1,2,3,4}
    };
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);

    // Connect the third wire
    agg.connect(wire3);

    // Push to three wires
    in1->pushAudio(&inLR, 4);
    in2->pushAudio(&inLR, 6);
    in3->pushAudio(&inLR, 8);

    // Pop to sum three wires
    std::fill(outL.begin(), outL.end(), 0);
    std::fill(outR.begin(), outR.end(), 0);
    sus_vec = agg.popAudio(&outLR, 10);
    EXPECT_EQ(sus_vec[0], 8);
    EXPECT_EQ(sus_vec[1], 8);
    ans = std::vector<std::vector<int>>{
        {3,6,9,12,10,12,7,8},
        {15,18,21,24,2,4,3,4}
    };
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);
}

TEST_F(AggregatorTest, AggregatePtr) {
    // Connect two wires to the pending list
    agg.connect(wire1);
    agg.connect(wire2);
    EXPECT_EQ(0, agg.getConnectedWireCount());

    std::shared_ptr<InJack<int, 2>> in1 = wire1->getInPtr();
    std::shared_ptr<InJack<int, 2>> in2 = wire2->getInPtr();
    std::shared_ptr<InJack<int, 2>> in3 = wire3->getInPtr();
    std::vector<int> ans {};
    std::vector<int> status{};

    // Push to two wires
    size_t sus{};
    std::vector<int> inR = vector<int>{5,6,7,8,1,2,3,4};

    in1->pushAudio(inR.data(), 4, 1);
    in2->pushAudio(inR.data(), 8, 1);

    // Pop to sum two wires
    sus = agg.popAudio(outR.data(), 10, 1);
    EXPECT_EQ(sus, 8);
    ans = std::vector<int>{10,12,14,16,1,2,3,4};
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);

    // Connect the third wire
    agg.connect(wire3);

    // Push to three wires
    in1->pushAudio(inR.data(), 4, 1);
    in2->pushAudio(inR.data(), 6, 1);
    in3->pushAudio(inR.data(), 8, 1);

    // Left channel should be empty
    std::fill(outR.begin(), outR.end(), 0);
    sus = agg.popAudio(outR.data(), 10, 0);
    EXPECT_EQ(sus, 0);

    // Pop to sum three wires
    std::fill(outR.begin(), outR.end(), 0);
    sus = agg.popAudio(outR.data(), 10, 1);
    EXPECT_EQ(sus, 8);
    ans = std::vector<int>{15,18,21,24,2,4,3,4};
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);
}