/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawTLU.h
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#ifndef RAWTLU_H
#define RAWTLU_H

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawTLUFormat.h>

namespace Belle2 {

  /**
   * The Raw TLU class
   * Class for data from DAQ PC for TLU(Trigger Logic Unit)
   * It is supposed to be used only in the DESY beam test
   */

  class RawTLU : public RawDataBlock {
  public:
    //! Default constructor
    RawTLU();

    //! Destructor
    virtual ~RawTLU();

    /*     //! Get # of words in this data packet */
    /*     int GetNwords(int n); */

    //!Get the size of the header
    int GetNwordsHeader(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetNwordsHeader(n);
    }

    //! Get Node ID
    unsigned int GetNodeID(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetNodeID(n);
    }

    //! Get Event #
    unsigned int GetEveNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetEveNo(n);
    }

    //! Get Magic Trailer #
    unsigned int GetMagicTrailer(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetMagicTrailer(n);
    }

    //! Get Run #
    int GetRunNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetRunNo(n);
    }

    //! Get TLU event tag
    unsigned int GetTLUEventTag(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTLUEventTag(n);
    }

    //! check data contents
    void CheckData(int n, unsigned int prev_evenum, unsigned int* cur_evenum)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->CheckData(n, prev_evenum, cur_evenum);
    }

    //! class to access
    RawTLUFormat* m_access; //! do not record

  protected :

    /// To derive from TObject
    ClassDef(RawTLU, 1);

  };
}

#endif
