//+
// File : RawDataBlock.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWDATABLOCK_H
#define RAWDATABLOCK_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <daq/dataobjects/RawHeader.h>
#include <daq/dataobjects/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawDataBlock : public TObject {
  public:
    //! Default constructor
    RawDataBlock();
    //! Constructor using existing pointer to raw data buffer
    RawDataBlock(int* bufin, int nwords);
    //! Destructor
    virtual ~RawDataBlock();

    //! Get total length of
    virtual int TotalBufNwords();

    //! get position of COPPER block
    virtual int GetBufferPos(int n);

    //! get nth buffer pointer
    virtual int* GetWholeBuffer();

    //! get nth buffer pointer
    virtual int GetNumEntries() { return m_num_events * m_num_nodes; }

    //! get nth buffer pointer
    virtual int GetNumNodes() { return m_num_nodes; }

    //! get nth buffer pointer
    virtual int GetNumEvents() { return m_num_events; }

    //! get nth buffer pointer
    virtual int* GetBuffer(int n);

    //! set buffer
    virtual void SetBuffer(int* bufin, int nwords, int malloc_flag, int num_events, int num_nodes);

    //! get COPPER Block Size
    virtual int GetBlockNwords(int n);


  protected :


    int m_nwords;
    int* m_buffer; //[m_nwords]
    bool m_use_prealloc_buf;

    RawHeader tmp_header;  //! Not record
    RawTrailer tmp_trailer; //! Not record

    int m_num_events;
    int m_num_nodes;

    ClassDef(RawDataBlock, 1);
  };
}

#endif
