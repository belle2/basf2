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
#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSWFormat.h>


namespace Belle2 {

  /**
   * The Raw FTSW class.
   *
   * This class stores the RAW data containing FTSW data(event #, trg time ).
   */
  class RawFTSW : public RawDataBlock {
  public:
    //! Default constructor
    RawFTSW();

    //! Constructor using existing pointer to raw data buffer
    //RawFTSW(int* bufin, int nwords);
    //! Destructor
    virtual ~RawFTSW();

    //! Get # of words of header
    int GetNwordsHeader(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetNwordsHeader(n);
    }

    //! Get Node # ( should be "TTD " )
    unsigned int GetFTSWNodeID(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetFTSWNodeID(n);
    }

    //! Get event #
    unsigned int GetEveNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetEveNo(n);
    }

    //! Get a word containing ctime and trigger type info
    unsigned int GetTTCtimeTRGType(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTTCtimeTRGType(n);
    }

    //! get unixtime of the trigger
    unsigned int GetTTUtime(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTTUtime(n);
    }

    //! Get ctime of the trigger
    int GetTTCtime(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTTCtime(n);
    }

    //! Get trgtype
    int GetTRGType(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTRGType(n);
    }

    //! Get timeval from ctime and utime
    void GetTTTimeVal(int n, struct timeval* tv)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetTTTimeVal(n, tv);
    }

    //! Get magic number for data corruption check
    unsigned int GetMagicTrailer(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetMagicTrailer(n);
    }

    //! check the data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->CheckData(n, prev_evenum, cur_evenum, prev_exprunsubrun_no, cur_exprunsubrun_no);
    }

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    unsigned int GetExpRunSubrun(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetExpRunSubrun(n);
    }

    //! Get run #
    int GetRunNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetRunNo(n);
    }


    //! Get subrun #
    int GetSubRunNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetSubRunNo(n);
    }

    //! get a word cotaining run # and subrun #
    int GetRunNoSubRunNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetRunNoSubRunNo(n);
    }

    //! Get Exp #
    int GetExpNo(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->GetExpNo(n);
    }

    //! DESY test only
    int Get15bitTLUTag(int n)
    {
      m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
      return m_access->Get15bitTLUTag(n);
    }

    //! class to access
    RawFTSWFormat* m_access; //! do not record

  protected :
    /// To derive from TObject
    /// ver.2 Remove m_FTSW_header and introduce a new data format on Nov. 20, 2013
    /// ver.3 Add m_access on Mar. 7 2016
    ClassDef(RawFTSW, 2);

  };
}

#endif
