//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

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
    bool receivedFlag[3];
}

bool VeinsInetSampleApplication::startApplication()
{
    // host[0] should stop at t=20s
    if (getParentModule()->getIndex() == 0) {
       auto callback = [this]() {
            getParentModule()->getDisplayString().setTagArg("i", 1, "red");
            traciVehicle->setSpeed(0);

            auto payload = makeShared<VeinsInetSampleMessage>();
            payload->setChunkLength(B(100));
            payload->setRoadId(traciVehicle->getRoadId().c_str());
            timestampPayload(payload);

            auto packet = createPacket("accident");
            packet->insertAtBack(payload);
            sendPacket(std::move(packet));

            // host should continue after 30s
            auto callback = [this]() {
                traciVehicle->setSpeed(-1);
            };
            timerManager.create(veins::TimerSpecification(callback).oneshotIn(SimTime(30, SIMTIME_S)));
        };
        timerManager.create(veins::TimerSpecification(callback).oneshotAt(SimTime(20, SIMTIME_S)));
    }
    // Schedule the  attack for host[1] at t=15s
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
    auto payload = pk->peekAtFront<VeinsInetSampleMessage>();

    EV_INFO << "Received packet: " << payload << endl;

    int currentVehicle = getParentModule()->getIndex();

    if (currentVehicle != 1 && receivedFlag[currentVehicle]) return;

    if (currentVehicle != 1 && (strcmp(pk->getName(), "Drop future packets") == 0)) {
        getParentModule()->getDisplayString().setTagArg("i", 1, "blue");
        receivedFlag[currentVehicle] = true;
        return;
    }

    if(strcmp(pk->getName(), "accident") == 0) {
        traciVehicle->changeRoute(payload->getRoadId(), 999.9);
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

    EV_INFO << "Starting Malicious attack" << endl;

    auto payload = makeShared<VeinsInetSampleMessage>();
    payload->setChunkLength(B(100));
    payload->setRoadId(traciVehicle->getRoadId().c_str());
    timestampPayload(payload);

    auto packet = createPacket("Drop future packets");
    packet->insertAtBack(payload);

    sendPacket(std::move(packet));

}


