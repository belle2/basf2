#include <iostream>
#include <string>
#include <framework/logging/Logger.h>
#include <ecl/dataobjects/ECLWaveformData.h>

using namespace std;
void Belle2::ECLWaveformData::print() const
{
  string line;
  for (size_t i = 0; i < c_nElements; ++ i) {
    line += to_string(m_matrixElement[i]);
    line += ' ';
  }
  B2INFO(line);
}
