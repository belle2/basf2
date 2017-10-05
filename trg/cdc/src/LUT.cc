//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : LUT.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to use LUTs for TRGCDC
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <sstream>
#include <fstream>
#include "trg/trg/Debug.h"
#include "trg/cdc/LUT.h"
//#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Track.h"
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

  TRGCDCLUT::TRGCDCLUT()
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
    if (TRGCDCLUT::dictionary.find(filename) == TRGCDCLUT::dictionary.end()) {
      TRGCDCLUT::dictionary[filename] = TRGCDCLUT();
      TRGCDCLUT::dictionary[filename].setDataFile(filename, nInputBit);
    }
    return &(TRGCDCLUT::dictionary[filename]);
  }
}

