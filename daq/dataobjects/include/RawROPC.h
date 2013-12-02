//+
// File : RawROPC.h
// Description : Module to handle raw data on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 16 - Aug - 2013
//-

#ifndef RAWROPC_H
#define RAWROPC_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawROPC : public TObject {
  public:
    //! Default constructor
    RawROPC();
    //! Constructor using existing pointer to raw data buffer
    //RawROPC(int* bufin, int nwords);
    //! Destructor
    virtual ~RawROPC();

    //! copy rawdata into internal buffer
    virtual void Copy(int* bufin, int nwords);

    //! allocate buffer
    virtual int* AllocateBuffer(int nwords);

    //! get buffer
    virtual int* GetBuffer();

    //! set buffer
    virtual void SetBuffer(int* bufin, int nwords, int malloc_flag);

    //! get data length
    virtual int GetBodyNwords();

    //! get data length
    virtual int Size();

    //! get data length
    virtual SendHeader* GetSendHeader();

    //! get data length
    virtual SendTrailer* GetSendTrailer();

    SendHeader m_header; //|| do not split the header
    SendTrailer m_trailer; //|| do not split the header

  private:
    int m_nwords;
    int* m_buffer; //[m_nwords]
    bool m_allocated;

    ClassDef(RawROPC, 1);
  };
}

#endif
