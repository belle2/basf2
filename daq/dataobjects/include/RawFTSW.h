//+
// File : RawFTSW.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWFTSW_H
#define RAWFTSW_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/RawDataBlock.h>
#include <daq/dataobjects/SendHeader.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawFTSW : public RawDataBlock {
  public:
    //! Default constructor
    RawFTSW();
    //! Constructor using existing pointer to raw data buffer
    RawFTSW(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSW();

    //! Copy nth header part to m_FTSW_header
    void SetFTSWHeader(int n);

  protected :

    SendHeader m_FTSW_header;


    ClassDef(RawFTSW, 1);
  };
}

#endif
