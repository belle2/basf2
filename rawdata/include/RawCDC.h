//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCDC_H
#define RAWCDC_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/RawCOPPER.h>
#include <rawdata/RawHeader.h>
#include <rawdata/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawCDC : public RawCOPPER {
  public:
    //! Default constructor
    RawCDC();
    //! Destructor
    virtual ~RawCDC();

    ClassDef(RawCDC, 1);
  };
}

#endif
