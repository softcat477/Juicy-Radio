
#include <gtest/gtest.h>
#include <vector>
#include <memory>

#include <Distributor.h>
#include <Wire.h>

using namespace std;

class DistributorTest : public ::testing::Test {
    protected:
    void SetUp() override {
        outL = vector<int>(256, 0);
        outR = vector<int>(256, 0);
        outLR = vector<int*> {outL.data(), outR.data()};
    }

    void TearDown() override {}

    // The wire itself
    WirePtr<int, 2> wire1 = make_shared<Wire<int, 2>>(2, 5);
    WirePtr<int, 2> wire2 = make_shared<Wire<int, 2>>(2, 5);
    WirePtr<int, 2> wire3 = make_shared<Wire<int, 2>>(2, 5);
    Distributor<int, 2> dist;

    // Returned buffer
    vector<int> outL;
    vector<int> outR;
    vector<int*> outLR;
};

TEST_F(DistributorTest, ConnectWire) {
    // Connect three wires to the pending list
    dist.connect(wire1);
    dist.connect(wire2);
    dist.connect(wire3);
    EXPECT_EQ(0, dist.getConnectedWireCount());
    EXPECT_EQ(3, dist.getPendingWireCount());

    // Push move pending wires to connected wires
    std::vector<int> inL = vector<int>{1,2,3,4,5,6,7,8};
    std::vector<int> inR = vector<int>{5,6,7,8,1,2,3,4};
    std::vector<int*> inLR = vector<int*>{inL.data(), inR.data()};
    std::vector<size_t> sus_vec{};
    sus_vec = dist.pushAudio(&inLR, 8);
    EXPECT_EQ(3, dist.getConnectedWireCount());
    EXPECT_EQ(0, dist.getPendingWireCount());
}

TEST_F(DistributorTest, DistributeVector) {
    // Connect two wires to the pending list
    dist.connect(wire1);
    dist.connect(wire2);

    std::shared_ptr<OutJack<int, 2>> out1 = wire1->getOutPtr();
    std::shared_ptr<OutJack<int, 2>> out2 = wire2->getOutPtr();
    std::shared_ptr<OutJack<int, 2>> out3 = wire3->getOutPtr();
    std::vector<std::vector<int>> ans {};
    std::vector<std::vector<int>> status{};

    std::vector<size_t> sus_vec{};
    std::vector<int> inL = vector<int>{1,2,3,4,5,6,7,8};
    std::vector<int> inR = vector<int>{5,6,7,8,1,2,3,4};
    std::vector<int*> inLR = vector<int*>{inL.data(), inR.data()};

    // Distribute to two wires
    sus_vec = dist.pushAudio(&inLR, 8);
    EXPECT_EQ(sus_vec[0], 8);
    EXPECT_EQ(sus_vec[1], 8);
    sus_vec = out1->popAudio(&outLR, 10);
    ans = std::vector<std::vector<int>>{
        {1,2,3,4,5,6,7,8},
        {5,6,7,8,1,2,3,4}
    };
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);
    sus_vec = out2->popAudio(&outLR, 10);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);

    // Connect the third wire
    dist.connect(wire3);

    // Distribute to three wires
    sus_vec = dist.pushAudio(&inLR, 5);
    EXPECT_EQ(sus_vec[0], 5);
    EXPECT_EQ(sus_vec[1], 5);
    sus_vec = out1->popAudio(&outLR, 10);
    ans = std::vector<std::vector<int>>{
        {1,2,3,4,5},
        {5,6,7,8,1}
    };
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);
    sus_vec = out2->popAudio(&outLR, 10);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);
    sus_vec = out3->popAudio(&outLR, 10);
    status = std::vector<std::vector<int>> {
        std::vector<int>(outLR[0], outLR[0] + sus_vec[0]),
        std::vector<int>(outLR[1], outLR[1] + sus_vec[1])
    };
    EXPECT_EQ(ans, status);
}

TEST_F(DistributorTest, DistributePtr) {
    // Connect two wires to the pending list
    dist.connect(wire1);
    dist.connect(wire2);

    std::shared_ptr<OutJack<int, 2>> out1 = wire1->getOutPtr();
    std::shared_ptr<OutJack<int, 2>> out2 = wire2->getOutPtr();
    std::shared_ptr<OutJack<int, 2>> out3 = wire3->getOutPtr();
    std::vector<int> ans {};
    std::vector<int> status{};

    size_t sus;
    std::vector<int> inR = vector<int>{5,6,7,8,1,2,3,4};

    // Distribute to two wires
    sus = dist.pushAudio(inR.data(), 8, 1);
    EXPECT_EQ(sus, 8);
    sus = out1->popAudio(outR.data(), 10, 1);
    ans = std::vector<int>{5,6,7,8,1,2,3,4};
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);
    sus = out2->popAudio(outR.data(), 10, 1);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);

    // Connect the third wire
    dist.connect(wire3);

    // Distribute to three wires
    sus = dist.pushAudio(inR.data(), 4, 1);
    EXPECT_EQ(sus, 4);
    sus = out1->popAudio(outR.data(), 10, 1);
    ans = std::vector<int>{5,6,7,8};
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);
    sus = out2->popAudio(outR.data(), 10, 1);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);
    sus = out3->popAudio(outR.data(), 10, 1);
    status = std::vector<int>(outR.data(), outR.data() + sus);
    EXPECT_EQ(ans, status);
}