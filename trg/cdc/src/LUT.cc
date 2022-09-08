/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to use LUTs for TRGCDC
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <fstream>
#include "trg/cdc/LUT.h"
#include "trg/cdc/TRGCDCTrack.h"
#include "trg/cdc/Link.h"
#include <cstdlib>
#include <math.h>

using namespace std;

namespace Belle2 {

  map<string, TRGCDCLUT> TRGCDCLUT::dictionary;

  std::string TRGCDCLUT::version(void) const
  {
    return string("TRGCDCLUT 1.00");
  }

  TRGCDCLUT::TRGCDCLUT() :
    m_data{}, m_bitsize(), m_name() // 2019/07/31 by ytlai
  {
  }

  TRGCDCLUT::~TRGCDCLUT()
  {
  }

  int TRGCDCLUT::getValue(unsigned id) const
  {
    unsigned range = pow(2, m_bitsize);
    if (id >= range) {
      return 0;
    } else {
      return m_data[id];
    }
  }

  void TRGCDCLUT::setDataFile(const string& filename, int nInputBit)
  {
    m_bitsize = nInputBit;
    m_name = filename;

    ifstream openFile;
    string tmpstr;
    int tmpint;
    int i = 0;
    int range = pow(2, nInputBit);
    openFile.open(filename.c_str());
    m_data.resize(range);
    while (getline(openFile, tmpstr) && i < range) {
      if (!(tmpstr.size() == 0)) {
        if (tmpstr[0] >= '0' && tmpstr[0] <= '9') {
          tmpint = atoi(tmpstr.c_str());
          m_data[i] = tmpint;
          i++;
        } else {
          continue;
        }
      }
    }
    openFile.close();
  }

  TRGCDCLUT* TRGCDCLUT::getLUT(const std::string& filename, int nInputBit)
  {
    if (!TRGCDCLUT::dictionary.count(filename)) {
      TRGCDCLUT::dictionary[filename] = TRGCDCLUT();
      TRGCDCLUT::dictionary[filename].setDataFile(filename, nInputBit);
    }
    return &(TRGCDCLUT::dictionary[filename]);
  }
}

