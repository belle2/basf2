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
#include <rawdata/dataobjects/RawFTSWFormat_v1.h>
#include <rawdata/dataobjects/RawFTSWFormat_v2.h>
#include <rawdata/dataobjects/RawFTSWFormat_latest.h>

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

    //! set buffer ( delete_flag : m_buffer is freeed( = 0 )/ not freeed( = 1 ) in Destructer )
    /* cppcheck-suppress missingOverride */
    void SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes) OVERRIDE_CPP17;

    //! read data, detect and set the version number of the data format
    void SetVersion();

    //! Check the version number of data format
    void CheckVersionSetBuffer();

    //! Get # of words of header
    int GetNwordsHeader(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetNwordsHeader(n);
    }

    //! Get Node # ( should be "TTD " )
    unsigned int GetFTSWNodeID(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetFTSWNodeID(n);
    }

    //! Get event #
    unsigned int GetEveNo(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetEveNo(n);
    }

    //! Get a word containing ctime and trigger type info
    unsigned int GetTTCtimeTRGType(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTTCtimeTRGType(n);
    }

    //! get unixtime of the trigger
    unsigned int GetTTUtime(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTTUtime(n);
    }

    //! Get ctime of the trigger
    int GetTTCtime(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTTCtime(n);
    }

    //! Get trgtype
    int GetTRGType(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTRGType(n);
    }

    //! Get timeval from ctime and utime
    void GetTTTimeVal(int n, struct timeval* tv)
    {
      CheckVersionSetBuffer();
      return m_access->GetTTTimeVal(n, tv);
    }

    //! Get timeval obtained from gettimeval()
    void GetPCTimeVal(int n, struct timeval* tv)
    {
      CheckVersionSetBuffer();
      return m_access->GetPCTimeVal(n, tv);
    }

    //! Get magic number for data corruption check
    unsigned int GetMagicTrailer(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetMagicTrailer(n);
    }

    //! check the data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
    {
      CheckVersionSetBuffer();
      return m_access->CheckData(n, prev_evenum, cur_evenum, prev_exprunsubrun_no, cur_exprunsubrun_no);
    }

    //! Exp# (10bit) run# (14bit) restart # (8bit)
    unsigned int GetExpRunSubrun(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetExpRunSubrun(n);
    }

    //! Get run #
    int GetRunNo(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetRunNo(n);
    }


    //! Get subrun #
    int GetSubRunNo(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetSubRunNo(n);
    }

    //! get a word cotaining run # and subrun #
    int GetRunNoSubRunNo(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetRunNoSubRunNo(n);
    }

    //! Get Exp #
    int GetExpNo(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetExpNo(n);
    }

    //! DESY test only
    int Get15bitTLUTag(int n)
    {
      CheckVersionSetBuffer();
      return m_access->Get15bitTLUTag(n);
    }

    //! HER injection = 1 or LER injection = 0
    int GetIsHER(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetIsHER(n);
    }

    //! Get time since the last injection
    unsigned int GetTimeSinceLastInjection(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTimeSinceLastInjection(n);
    }

    //! Get time since the previous trigger
    unsigned int GetTimeSincePrevTrigger(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetTimeSincePrevTrigger(n);
    }

    //! Get a bunch number
    unsigned int GetBunchNumber(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetBunchNumber(n);
    }

    //! Get a frame count
    unsigned int GetFrameCount(int n)
    {
      CheckVersionSetBuffer();
      return m_access->GetFrameCount(n);
    }

    enum {
      /// Added to distinguish different version.
      /// ver.0 : early DESY-beam-test version( Node ID 0x545444** is at 5th word (0,1,2,..5)
      ///       : Please ask Nakao-san about his e-mail ( [daq-experts:8055] FEE data header and FTSW data format)
      /// ver.1 : late DESY-beam-test version( Node ID 0x545444** was moved to 6th word (0,1,2,..5)
      ///       : header size is ? words ( I need to ask Nakao-san because it will be used for distinguish between 0,1, and 2
      /// ver 2 : phase II ? version
      ///       : header size is 8 words
      ///       : Format is written by Nakao-san in ver.26(as of 2019.3.1 ) of https://confluence.desy.de/pages/viewpage.action?spaceKey=BI&title=DAQ+TimingDistribution#DAQTimingDistribution-InformationsentfromTTDtodatastream
      //! 0x545452?? up to ver.2 this number is more or less same. So, it cannot be used to distinguish different version numbers
      POS_NODE_FORMAT_ID = 6,  // Position of format ID in header
      FTSW_FORMAT_MASK = 0x000000FF,

      FORMAT_ID_VER_0TO2 = 0x54544420,
      FORMAT_ID_VER_0TO3 = 0x54544431,

      POS_HEADER_SIZE = 1, //! The same number of this information must appear in RawFTSWFormat*.h. Information should be placed in one place but I need to put this number in RawFTSW.h because it is used to distinguish different version numbers.
      // header size is used to distinguish different version number for ver. 0, 1, and 2
      VER_0_HEADER_SIZE = 0, //! Unpacker for ver.0(early DESY version) is not available.
      VER_1_HEADER_SIZE = 0, // ! Unpacker for ver.1(late DESY version) is available but it is unlikely for a user to read this ver. Until there is a request to read ver.1, it is closed.
      VER_2_HEADER_SIZE = 8,
      VER_3_HEADER_SIZE = 8
    };

    //! class to access
    RawFTSWFormat* m_access{}; //! do not record

    /// Version of the format
    int m_version = -1; //! do not record

  protected :
    /// To derive from TObject
    /// ver.2 Remove m_FTSW_header and introduce a new data format on Nov. 20, 2013
    /// ver.3 Add m_access on Mar. 7 2016 but Classded stays 2 for some(?) reason
    /// ver.4 Add m_version on Feb. 18 2019 for the new format version defined by Nakao-san
    /// Reason to stay at *2* is that the persistent content did *not change*
    /// If necessary: be aware that we have to change this in online and offline at the same time!
    ClassDefOverride(RawFTSW, 2);
  };

  inline void RawFTSW::CheckVersionSetBuffer()
  {
    if (m_version < 0  || m_access == NULL) {
      // Since both ver.0, 1 and ver.2 will show m_buffer[ POS_NODE_ID ] & FORMAT_MASK == 0x0, I need to ignore the check for ver.0 and 1
      SetVersion();
    }
    m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
  }

}

#endif
