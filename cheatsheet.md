# VEINS cheatsheet

Functions are global and static for every node. This means, that every node uses the same `processPacket()` function, for example. You have to filter which vehicle does which action inside the funcitons! Also note that messages are always "broadcast".

If you're using global variables (flags), don't forget to add them to 
`VeinsInetSampleApplication::VeinsInetSampleApplication()` and the `.h` (header) file!

---

### Attacker: Node[1]

## Task 1: Fake News!
Send a false information message to the other vehicles. Let the attacker inform the other vehciles about a fake accident ahead. If they receive that packet, they should change the route, otherwise, just do nothing.

## Task 2: Red Flag
Send a packet which should (kind of) simulate a DOS attack:
The attacker sends out one packet: if the other vehicles receive that packet, they should drop future incoming packets - even if there is an actual accident ahead (this isn't 100% techincally correct, but close enough in Veins). Hint: use a flag.

---

Get current vehicle: `getParentModule()->getIndex();`

Get packet name: `pk->getName()`
Compare packet name to string: `strcmp(pk->getName(), "string") == 0`

Change vehicle color (to blue): 
`getParentModule()->getDisplayString().setTagArg("i", 1, "blue")`

create payload: 
`auto payload = makeShared<VeinsInetSampleMessage>();`
`payload->setChunkLength(B(100));`
`payload->setRoadId(traciVehicle->getRoadId().c_str());`

create and send packet:
`auto packet = createPacket("packetName");`
`packet.>insertAtBack(payload);`
`sendPacket(std::move(packet);`

stop vehicle:
`traciVehicle->setSpeed(0)`

Log stuff:
`EV_INFO << stuff;`

Flag for each vehicle:
`bool flags[amount of vehicles]`

Example:
Node 2 executes an attack 5 seconds into the simulation:
`if(getParentModule()->getIndex() == 1) {
auto malCallback = [this]() {
MaliciousAttack();
};
}timerManager.create(veins::TimerSpecification(malCallback).oneshotAt(simTime(20, SIMTIME_S)));
`

---

### Task 3: Trust Issues (bonus)
To counteract some of the attacks, we could ask ourselves whether you can trust certain vehicles or not.
Only accept packets from vehicles that are trusted.
(You will need to add a variable, e.g. boolean, to `VeinsInetCar.ned` and assign a value to the specific nodes in  the `omnetpp.ini` file. Next, create getters and setters in `VeinsInetSampleMessage_m.cc/.h`. You can use those functions with the payload in `VeinsInetSampleApplication`)

