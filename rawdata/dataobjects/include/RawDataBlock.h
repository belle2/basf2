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
#include <framework/datastore/DataStore.h>
#include <TObject.h>


namespace Belle2 {

  /**
   * The RawDataBlock class
   * Base class for rawdata handling
   */
  class RawDataBlock : public TObject {
  public:
    //! Default constructor
    RawDataBlock();
    //! Constructor using existing pointer to raw data buffer
//    RawDataBlock(int* bufin, int nwords);
    //! Destructor
    virtual ~RawDataBlock();

    //! set buffer ( malloc_flag : m_buffer is freeed( = 0 )/ not freeed( = 1 ) in Destructer )
    virtual void SetBuffer(int* bufin, int nwords, int malloc_flag, int num_events, int num_nodes);

    //! Get total length of m_buffer
    virtual int TotalBufNwords();

    //! get position of data block in word
    virtual int GetBufferPos(int n);

    //! get nth buffer pointer
    virtual int* GetBuffer(int n);

    //! get pointer to  buffer(m_buffer)
    virtual int* GetWholeBuffer();

    //! get # of data blocks = (# of nodes)*(# of events)
    virtual int GetNumEntries() { return m_num_events * m_num_nodes; }

    //! get # of data sources(e.g. # of COPPER boards) in m_buffer
    virtual int GetNumNodes() { return m_num_nodes; }

    //! get # of events in m_buffer
    virtual int GetNumEvents() { return m_num_events; }

    //! get malloc_flag
    virtual int GetPreAllocFlag() { return m_use_prealloc_buf; }

    //! get size of a data block
    virtual int GetBlockNwords(int n);


    //! get FTSW ID to check whether this data block is FTSW data or not
    virtual int CheckFTSWID(int n);

    //! get FTSW ID to check whether this data block is FTSW data or not
    virtual int CheckTLUID(int n);

    //! Copy one datablock to buffer
    virtual void CopyBlock(int n, int* buf_to);

    //! print data
    virtual void PrintData(int* buf, int nwords);

    enum {
      POS_NWORDS = 0,
      POS_NODE_ID = 6
    };

    enum {
      // Tentatively needed to distinguish new and old FTSW format, which will be changed in Nov. 2013
      POS_FTSW_ID_OLD = 5,
      TEMP_POS_NWORDS_HEADER = 1,
      OLD_FTSW_NWORDS_HEADER = 6
    };

  protected :
    /// number of words of buffer
    int m_nwords;

    /// number of nodes in this object
    int m_num_nodes;

    /// number of events in this object
    int m_num_events;

    /// Buffer
    int* m_buffer; //[m_nwords]

    //! flag for deleting m_buffer in destructer(1:delete, 0: not delete)
    int m_use_prealloc_buf;

    //! no longer used
    int m_which_part;

    /// To drive from TObject
    ClassDef(RawDataBlock, 1);

  };

}

#endif
