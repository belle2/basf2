//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWARICH_H
#define RAWARICH_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw ARICH class
   * Class for RawCOPPER class data taken by ARICH
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawARICH : public RawCOPPER {
  public:
    //! Default constructor
    RawARICH();
    //! Destructor
    virtual ~RawARICH();
    /// To derive from TObject
    ClassDef(RawARICH, 1);
  };
}

#endif
