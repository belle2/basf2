//+
// File : RawPXD.h
// Description : Module to handle raw data on PXD
//
// Author : Tomoyuki Konno, Tokyo Metropolitan University
// Date : 21 - Oct - 2013
//-

#ifndef RAWPXD_H
#define RAWPXD_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawPXD : public TObject {
  public:
    //! Default constructor
    RawPXD();
    //! Constructor using existing pointer to raw data buffer
    RawPXD(int* bufin, int nwords);
    //! Destructor
    virtual ~RawPXD();

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

    ClassDef(RawPXD, 1);
  };
}

#endif
