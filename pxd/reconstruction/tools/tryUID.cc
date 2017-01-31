#include <vxd/dataobjects/VxdID.h>
#include <iostream>
#include <bitset>

using namespace std;
using namespace Belle2;

int main(int, char**)
{
  short layer = 2;
  bitset<8> bLayer(layer);
  cout << "Layer: " << layer << " " << bLayer << endl;
  short ladder = 3;
  bitset<8> bLadder(ladder);
  cout << "Ladder: " << ladder << " " << bLadder << endl;
  short sensor = 1;
  bitset<8> bSensor(sensor);
  cout << "Sensor: " << sensor << " " << bSensor << endl;
  short u = 232;
  bitset<16> bU(u);
  cout << "u: " << u << " " << bU << endl;
  short v = 714;
  bitset<16> bV(v);
  cout << "v: " << v << " " << bV << endl;
  VxdID sensorID(layer, ladder, sensor);
  sensorID.setSegmentNumber(1);
  sensorID.setSegmentNumber(sensorID.getSegmentNumber() << 2);
  bitset<16> bSensorID(sensorID);
  cout << "sensorID: " << sensorID << " " << bSensorID << endl;
  int UID = v + (u << 10) + (sensorID << 16);
  bitset<32> bUID(UID);
  cout << "UID: " << UID << " " << bUID << endl;
  return 0;
}
