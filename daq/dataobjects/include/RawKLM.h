//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWKLM_H
#define RAWKLM_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/RawHeader.h>
#include <daq/dataobjects/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawKLM : public RawCOPPER {
  public:
    //! Default constructor
    RawKLM();
    //! Destructor
    virtual ~RawKLM();

    ClassDef(RawKLM, 1);
  };
}

#endif
