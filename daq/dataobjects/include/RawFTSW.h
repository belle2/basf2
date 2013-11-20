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
#include <daq/dataobjects/SendTrailer.h>
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

    //!
    int GetNwords(int n);

    //!
    int GetNwordsHeader(int n);

    //!
    int GetFTSWNodeID(int n);

    //!
    int GetTrailerMagic(int n);

    //!
    unsigned int GetFTSWData1(int n);

    //!
    unsigned int GetFTSWData2(int n);

    //!
    double GetEventUnixTime(int n);

    enum {
      POS_FTSW1 = 0,
      POS_FTSW2 = 1,
      SIZE_FTSW_BODY = 4 // modified by Oct.3, 2013, Nakao-san's New firmware?
    };

  protected :
    Belle2::SendHeader m_FTSW_header; //

    ClassDef(RawFTSW, 1);
  };
}

#endif
