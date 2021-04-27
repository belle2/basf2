//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWECL_H
#define RAWECL_H

// Includes
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /**
   * The Raw ECL class
   * Class for RawCOPPER class data taken by ECL
   * Currently, this class is almost same as RawCOPPER class.
   */
  class RawECL : public RawCOPPER {
  public:
    //! Default constructor
    RawECL();
    //! Destructor
    virtual ~RawECL();
    /// To derive from TObject
    ClassDef(RawECL, 1);
  };
}

#endif
