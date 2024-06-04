#include "veins_inet/VeinsInetSampleApplication.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "veins_inet/VeinsInetSampleMessage_m.h"

using namespace inet;

Define_Module(VeinsInetSampleApplication);

VeinsInetSampleApplication::VeinsInetSampleApplication()
{
    bool bogusReceived[3]; // Initialize flags for each vehicle
}

bool VeinsInetSampleApplication::startApplication()
{

    // host[0] should stop at t=20s
    // if (getParentModule()->getIndex() == 0) {
        // auto callback = [this]() {
            // getParentModule()->getDisplayString().setTagArg("i", 1, "red");

            // traciVehicle->setSpeed(0);

            // auto payload = makeShared<VeinsInetSampleMessage>();
            // payload->setChunkLength(B(100));
            // payload->setRoadId(traciVehicle->getRoadId().c_str());
            // timestampPayload(payload);

            // auto packet = createPacket("accident");
            // packet->insertAtBack(payload);
            // sendPacket(std::move(packet));

            // host should continue after 30s
            // auto callback = [this]() {
                // traciVehicle->setSpeed(-1);
            // };
            // timerManager.create(veins::TimerSpecification(callback).oneshotIn(SimTime(30, SIMTIME_S)));
        // };
        // timerManager.create(veins::TimerSpecification(callback).oneshotAt(SimTime(20, SIMTIME_S)));
    // }

    // Schedule the DoS attack for host[1] at t=25s
    if (getParentModule()->getIndex() == 1) {
        auto malCallback = [this]() {
            MaliciousAttack();
        };
        timerManager.create(veins::TimerSpecification(malCallback).oneshotAt(SimTime(15, SIMTIME_S)));
    }

    return true;
}

bool VeinsInetSampleApplication::stopApplication()
{
    return true;
}

VeinsInetSampleApplication::~VeinsInetSampleApplication()
{
}

void VeinsInetSampleApplication::processPacket(std::shared_ptr<inet::Packet> pk)
{
    
    // get current vehicle index and trust
    int currentVehicle = getParentModule()->getIndex();
    bool trustLevel = payload->getTrustLevel();

    // get packet payload
    auto payload = pk->peekAtFront<VeinsInetSampleMessage>();
    EV_INFO << "Received packet: " << payload << endl;


    // change route if victim vehicle and bogus packet is received
    if (currentVehicle != 1 && (strcmp(pk->getName(), "BogusAccident") == 0 || bogusReceived[currentVehicle] == true)) {            
        // visualize
        getParentModule()->getDisplayString().setTagArg("i", 1, "blue");

        if(bogusReceived[currentVehicle]) return;

        auto packet = createPacket("relay");
        packet->insertAtBack(payload);
        sendPacket(std::move(packet));
        
        // change route
        traciVehicle->changeRoute(payload->getRoadId(), 999.9);

        bogusReceived[currentVehicle] = true;
        return;
    }

    // only accept packets from trusted vehicles
    if(trustLevel) {
        
        getParentModule()->getDisplayString().setTagArg("i", 1, "green");
        if (haveForwarded) return;
        auto packet = createPacket("relay");
        packet->insertAtBack(payload);
        sendPacket(std::move(packet));
        haveForwarded = true;
        return;
    }
}

void VeinsInetSampleApplication::MaliciousAttack()
{

    bool trustLevel = getParentModule()->par("trustLevel").boolValue();
    int currentVehicle = getParentModule()->getIndex();

    EV_INFO << trustLevel << endl;

    // craete payload with no trust
    auto payload = makeShared<VeinsInetSampleMessage>();
    payload->setChunkLength(B(100));
    payload->setIsTrusted(trustLevel);
    timestampPayload(payload);
    
    payload->setRoadId(traciVehicle->getRoadId().c_str());
    
    // create packet
    auto packet = createPacket("BogusAccident");
    packet->insertAtBack(payload);

    EV_INFO << "Launching Bogus attack with packet: " << endl;

    sendPacket(std::move(packet));
    
}

