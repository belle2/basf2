//+
// File : RawTLU.h
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#ifndef RAWTLU_H
#define RAWTLU_H

// Includes
#include <rawdata/dataobjects/RawFTSW.h>

namespace Belle2 {

  class RawTLU : public RawFTSW {
  public:
    //! Default constructor
    RawTLU();

    //! Destructor
    virtual ~RawTLU();

  protected :

    ClassDef(RawTLU, 1);

  };
}

#endif
