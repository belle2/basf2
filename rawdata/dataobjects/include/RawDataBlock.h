/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
#include <TObject.h>
#include <rawdata/dataobjects/RawDataBlockFormat.h>


namespace Belle2 {

  /**
   * The RawDataBlock class
   * Base class for rawdata handling
   */
  class RawDataBlock : public TObject {
  public:
    //! Default constructor
    RawDataBlock();

    //! Destructor
    virtual ~RawDataBlock();

    //! set buffer ( delete_flag : m_buffer is freeed( = 0 )/ not freeed( = 1 ) in Destructer )
    virtual void SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes);

    //! Get total length of m_buffer
    virtual int TotalBufNwords()
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->TotalBufNwords();
    }

    //! get position of data block in word
    virtual int GetBufferPos(int n)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetBufferPos(n);
    }

    //! get nth buffer pointer
    virtual int* GetBuffer(int n)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetBuffer(n);
    }

    //! get pointer to  buffer(m_buffer)
    virtual int* GetWholeBuffer()
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetWholeBuffer();
    }

    //! get # of data blocks = (# of nodes)*(# of events)
    virtual int GetNumEntries()
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetNumEntries();
    }

    //! get # of data sources(e.g. # of COPPER boards) in m_buffer
    virtual int GetNumNodes()
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetNumNodes();
    }

    //! get # of events in m_buffer
    virtual int GetNumEvents()
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetNumEvents();
    }

    //! get malloc_flag
    virtual int GetPreAllocFlag()
    {
      return m_use_prealloc_buf;
    }

    //! get size of a data block
    virtual int GetBlockNwords(int n)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->GetBlockNwords(n);
    }

    //! get FTSW ID to check whether this data block is FTSW data or not
    virtual int CheckFTSWID(int n)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->CheckFTSWID(n);
    }

    //! get FTSW ID to check whether this data block is FTSW data or not
    virtual int CheckTLUID(int n)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->CheckTLUID(n);
    }

    //! Copy one datablock to buffer
    virtual void CopyBlock(int n, int* buf_to)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->CopyBlock(n, buf_to);
    }

    //! print data
    virtual void PrintData(int* buf, int nwords)
    {
      m_access_dblk->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access_dblk->PrintData(buf, nwords);
    }

    /*     enum { */
    /*       POS_NWORDS = 0, */
    /*       POS_NODE_ID = 6 */
    /*     }; */

    /*     enum { */
    /*       // Tentatively needed to distinguish new and old FTSW format, which will be changed in Nov. 2013 */
    /*       POS_FTSW_ID_OLD = 5, */
    /*       TEMP_POS_NWORDS_HEADER = 1, */
    /*       OLD_FTSW_NWORDS_HEADER = 6 */
    /*     }; */

    //! class to access
    RawDataBlockFormat* m_access_dblk; //! do not record

  protected :
    /// number of words of buffer
    int m_nwords;

    /// number of nodes in this object
    int m_num_nodes;

    /// number of events in this object
    int m_num_events;

    /// Buffer
    int* m_buffer; //[m_nwords]

    //! flag for deleting m_buffer in destructer( 0:delete, 1: not delete)
    //! When using pre-allocated buffer, the buffer should be reused and not deleted in the destructer
    int m_use_prealloc_buf; //! not record

    /// To drive from TObject
    ClassDef(RawDataBlock, 3);
  };

}

#endif
