#include <pxd/dataobjects/PXDCalibrationMap.h>
#include <iostream>

using namespace std;
using namespace Belle2;

int main(int, char**)
{
  PXDCalibrationMap calmap;
  calmap.setValue(1, 2, 3, 4, 5, 3.1415926);
  cout << calmap.getValue(1, 2, 3, 4, 5) << endl;
  cout << calmap.getValue(0, 0, 0, 0, 0) << endl;
  return 0;
}
