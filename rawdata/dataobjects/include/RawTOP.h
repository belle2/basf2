//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWTOP_H
#define RAWTOP_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw TOP class
   * Class for RawCOPPER class data taken by TOP
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawTOP : public RawCOPPER {
  public:
    //! Default constructor
    RawTOP();
    //! Destructor
    virtual ~RawTOP();

    /// To derive from TObject
    ClassDef(RawTOP, 1);
  };
}

#endif
