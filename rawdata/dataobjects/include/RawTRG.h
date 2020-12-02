//+
// File : RawTRG.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWTRG_H
#define RAWTRG_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw TOP class
   * Class for RawCOPPER class data taken by TOP
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawTRG : public RawCOPPER {
  public:
    //! Default constructor
    RawTRG();
    //! Destructor
    virtual ~RawTRG();

    /// To derive from TObject
    ClassDef(RawTRG, 1);
  };
}

#endif
