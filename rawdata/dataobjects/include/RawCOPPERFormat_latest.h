/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawCOPPERFormat_latest.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCOPPERFORMAT_LATEST_H
#define RAWCOPPERFORMAT_LATEST_H

// Includes
#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <rawdata/dataobjects/RawHeader_latest.h>
#include <rawdata/dataobjects/RawTrailer_latest.h>
#include <rawdata/CRCCalculator.h>
/* #include <framework/datastore/DataStore.h> */
/* #include <TObject.h> */

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

namespace Belle2 {

  /**
   * The Raw COPPER class ver.1 ( the latest version since May, 2014 )
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */

  class RawCOPPERFormat_latest : public RawCOPPERFormat {
  public:
    //! Default constructor
    RawCOPPERFormat_latest();

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPERFormat_latest(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPERFormat_latest();

    //
    // Get position of or pointer to data
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////


    //! get Detector buffer length of slot A
    /* cppcheck-suppress missingOverride */
    int Get1stDetectorNwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector buffer length of slot B
    /* cppcheck-suppress missingOverride */
    int Get2ndDetectorNwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector buffer length of slot C
    /* cppcheck-suppress missingOverride */
    int Get3rdDetectorNwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector buffer length of slot D
    /* cppcheck-suppress missingOverride */
    int Get4thDetectorNwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector buffer of slot A
    /* cppcheck-suppress missingOverride */
    int* Get1stDetectorBuffer(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector Buffer of slot B
    /* cppcheck-suppress missingOverride */
    int* Get2ndDetectorBuffer(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector Buffer of slot C
    /* cppcheck-suppress missingOverride */
    int* Get3rdDetectorBuffer(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get Detector Buffer of slot D
    /* cppcheck-suppress missingOverride */
    int* Get4thDetectorBuffer(int n) OVERRIDE_CPP17 FINAL_CPP17;

    ///////////////////////////////////////////////////////////////////////////////////////

    //! get buffer pointer of rawcopper trailer
    /* cppcheck-suppress missingOverride */
    int* GetRawTrlBufPtr(int n) OVERRIDE_CPP17;

    //
    // Get information from "RawCOPPERFormat_latest header" attached by DAQ software
    //

    /* cppcheck-suppress missingOverride */
    int GetExpNo(int n) OVERRIDE_CPP17;    //! get Experimental # from header

    /* cppcheck-suppress missingOverride */
    unsigned int GetExpRunSubrun(int n) OVERRIDE_CPP17;    //! Exp# (10bit) run# (14bit) restart # (8bit)

    /* cppcheck-suppress missingOverride */
    int GetRunNo(int n) OVERRIDE_CPP17;    //! get run # (14bit)

    /* cppcheck-suppress missingOverride */
    int GetSubRunNo(int n) OVERRIDE_CPP17;    //! get subrun #(8bit)

    /* cppcheck-suppress missingOverride */
    unsigned int GetEveNo(int n) OVERRIDE_CPP17;    //! get contents of header

    /* cppcheck-suppress missingOverride */
    int GetDataType(int n) OVERRIDE_CPP17;    //! get contents of header

    /* cppcheck-suppress missingOverride */
    int GetTruncMask(int n) OVERRIDE_CPP17;    //! get contents of header

    //! Get Detected Error bitflag
    /* cppcheck-suppress missingOverride */
    unsigned int GetErrorBitFlag(int n) OVERRIDE_CPP17;

    //! Add Detected Error bitflag
    /* cppcheck-suppress missingOverride */
    void AddErrorBitFlag(int n, unsigned int error_bit_flag) OVERRIDE_CPP17;

    //! check CRC packet Error
    /* cppcheck-suppress missingOverride */
    int GetPacketCRCError(int n) OVERRIDE_CPP17;

    //! check CRC event Error
    /* cppcheck-suppress missingOverride */
    int GetEventCRCError(int n) OVERRIDE_CPP17;

    //! get node-ID from data
    /* cppcheck-suppress missingOverride */
    unsigned int GetNodeID(int n) OVERRIDE_CPP17;

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //

    //! get # of FINNESEs which contains data
    /* cppcheck-suppress missingOverride */
    virtual int GetNumFINESSEBlock(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE slot A buffer
    /* cppcheck-suppress missingOverride */
    int Get1stFINESSENwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get data size of  FINESSE slot B buffer
    /* cppcheck-suppress missingOverride */
    int Get2ndFINESSENwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get data size of  FINESSE slot C buffer
    /* cppcheck-suppress missingOverride */
    int Get3rdFINESSENwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get data size of  FINESSE slot D buffer
    /* cppcheck-suppress missingOverride */
    int Get4thFINESSENwords(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get # of offset words for FINESSE slot A buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset1stFINESSE(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get # of offset words for FINESSE slot B buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset2ndFINESSE(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get # of offset words for FINESSE slot C buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset3rdFINESSE(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //! get # of offset words for FINESSE slot D buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset4thFINESSE(int n) OVERRIDE_CPP17 FINAL_CPP17;

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    //    virtual int* GetFTSW2Words(int n);

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    //! Check B2LFEE header version
    /* cppcheck-suppress missingOverride */
    void CheckB2LFEEHeaderVersion(int  n) OVERRIDE_CPP17;
#endif

    //! Check if COPPER Magic words are correct
    /* cppcheck-suppress missingOverride */
    unsigned int GetTTCtimeTRGType(int n) OVERRIDE_CPP17;

    //! Check if COPPER Magic words are correct
    /* cppcheck-suppress missingOverride */
    unsigned int GetTTUtime(int n) OVERRIDE_CPP17;

    /*     //! calc COPPER driver's checksum value */
    /*     virtual unsigned int CalcDriverChkSum(int n) = 0; */


    //! Get ctime
    /* cppcheck-suppress missingOverride */
    int GetTTCtime(int n) OVERRIDE_CPP17;

    //! Get trgtype
    /* cppcheck-suppress missingOverride */
    int GetTRGType(int n) OVERRIDE_CPP17;

    //! Get timeval
    /* cppcheck-suppress missingOverride */
    void GetTTTimeVal(int n, struct timeval* tv) OVERRIDE_CPP17;

    //! Get the max number of channels in a readout board
    /* cppcheck-suppress missingOverride */
    int GetMaxNumOfCh(int n) OVERRIDE_CPP17;

    //! Get a pointer to a FINESSE buffer
    /* cppcheck-suppress missingOverride */
    int* GetFINESSEBuffer(int n, int finesse_num) OVERRIDE_CPP17;

    //! Get the size of a finesse buffer
    /* cppcheck-suppress missingOverride */
    int GetFINESSENwords(int n, int finesse_num) OVERRIDE_CPP17;

    //! Check header value of all channels
    /* cppcheck-suppress missingOverride */
    void CompareHeaderValue(int n, const unsigned int (&input_val)[MAX_PCIE40_CH] ,
                            std::vector<std::vector< unsigned int>>& summary_table) OVERRIDE_CPP17;

    /// Format version number
    enum {
      DATA_FORMAT_VERSION = 4
    };

    //
    // size of "COPPER front header" and "COPPER trailer" (Common for pre/post format)
    //
    enum {
      SIZE_COPPER_DRIVER_HEADER = 0,
      SIZE_COPPER_DRIVER_TRAILER = 0
    };

    // Data Format : "COPPER header"
    enum {
      SIZE_COPPER_HEADER = 0
    };

    // Data Format : "COPPER Trailer"
    enum {
      SIZE_COPPER_TRAILER = 0
    };

    //! header ( not recorded )
    RawHeader_latest tmp_header;

    //! trailer ( not recorded )
    RawTrailer_latest tmp_trailer;


  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(RawCOPPERFormat_latest, 2);
  };




  inline int* RawCOPPERFormat_latest::GetRawTrlBufPtr(int n)
  {
    int pos_nwords;
    if (n == (m_num_events * m_num_nodes) - 1) {
      pos_nwords = m_nwords - tmp_trailer.GetTrlNwords();
    } else {
      pos_nwords = GetBufferPos(n + 1) - tmp_trailer.GetTrlNwords();
    }
    return &(m_buffer[ pos_nwords ]);
  }

  inline int RawCOPPERFormat_latest::GetExpNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpNo();
  }


  inline unsigned int RawCOPPERFormat_latest::GetExpRunSubrun(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpRunSubrun();
  }


  inline int RawCOPPERFormat_latest::GetRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetRunNo();
  }


  inline int RawCOPPERFormat_latest::GetSubRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetSubRunNo();
  }

  inline unsigned int RawCOPPERFormat_latest::GetEveNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetEveNo();
  }


  inline unsigned int RawCOPPERFormat_latest::GetNodeID(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetNodeID();
  }


  inline int RawCOPPERFormat_latest::GetDataType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetDataType();
  }

  inline int RawCOPPERFormat_latest::GetTruncMask(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTruncMask();
  }

  inline unsigned int RawCOPPERFormat_latest::GetErrorBitFlag(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetErrorBitFlag();
  }

  inline void RawCOPPERFormat_latest::AddErrorBitFlag(int n, unsigned int error_bit_flag)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.AddErrorBitFlag(error_bit_flag);
    return;
  }

  inline int RawCOPPERFormat_latest::GetPacketCRCError(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetPacketCRCError();
  }

  inline int RawCOPPERFormat_latest::GetEventCRCError(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetEventCRCError();
  }

  inline unsigned int RawCOPPERFormat_latest::GetTTCtimeTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtimeTRGType();
  }

  inline unsigned int RawCOPPERFormat_latest::GetTTUtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTUtime();
  }

  inline int RawCOPPERFormat_latest::GetTTCtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtime();
  }

  inline int RawCOPPERFormat_latest::GetTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTRGType();
  }

  inline void RawCOPPERFormat_latest::GetTTTimeVal(int n, struct timeval* tv)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.GetTTTimeVal(tv);
    return ;
  }

  inline int RawCOPPERFormat_latest::GetMaxNumOfCh(int/* n */)
  {
    return MAX_PCIE40_CH;
  }

  inline int RawCOPPERFormat_latest::GetFINESSENwords(int n, int finesse_num)
  {

    // check if finesse_num is in a range
    if (finesse_num < 0 || finesse_num >= MAX_PCIE40_CH) {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] Invalid finesse # (=%d): %s %s %d\n", finesse_num,
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("[DEBUG] %s\n", err_buf);
      B2FATAL(err_buf);
      return 0;
    }

    int pos_nwords = GetBufferPos(n) + (tmp_header.POS_CH_POS_TABLE + finesse_num);
    int nwords = 0;

    if (finesse_num == (MAX_PCIE40_CH - 1)) {
      nwords = GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - m_buffer[ pos_nwords ];
    } else {
      nwords = m_buffer[ pos_nwords + 1 ] - m_buffer[ pos_nwords ];
    }

    if (nwords < 0 || nwords > 1e6) {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] ERROR_EVENT : # of words is strange. %d (ch=%d) : eve 0x%x exp %d run %d sub %d\n %s %s %d\n",
              nwords, finesse_num,
              GetEveNo(n), GetExpNo(n), GetRunNo(n), GetSubRunNo(n),
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("[DEBUG] %s\n", err_buf);
      B2FATAL(err_buf);
    }
    return nwords;
  }

  inline int* RawCOPPERFormat_latest::GetFINESSEBuffer(int n, int finesse_num)
  {
    if (finesse_num >= MAX_PCIE40_CH || finesse_num < 0) {
      char err_buf[500];
      sprintf(err_buf, "[FATAL] Invalid finesse # : %s %s %d\n",
              __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("[DEBUG] %s\n", err_buf);
      B2FATAL(err_buf);
      return NULL;
    }

    if (GetFINESSENwords(n, finesse_num) > 0) {
      int cur_pos = GetBufferPos(n)
                    + m_buffer[ GetBufferPos(n) + (tmp_header.POS_CH_POS_TABLE + finesse_num) ];

      if (m_nwords <= 0 || cur_pos >= m_nwords) {
        char err_buf[500];
        sprintf(err_buf, "[FATAL] The position of the buffer( block %d, ch %d) is out of bounds (m_nwords = %d) : %s %s %d\n",
                n, finesse_num, m_nwords,
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("[DEBUG] %s\n", err_buf);
        B2FATAL(err_buf);
        return NULL;
      }
      return (m_buffer + cur_pos);
    }
    return NULL;
  }

}
#endif
