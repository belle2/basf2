/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWCOPPERFORMAT_V1_H
#define RAWCOPPERFORMAT_V1_H

// Includes
#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <rawdata/dataobjects/RawHeader_v1.h>
#include <rawdata/dataobjects/RawTrailer_v1.h>
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
  class RawCOPPERFormat_v1 : public RawCOPPERFormat {
  public:
    //! Default constructor
    RawCOPPERFormat_v1();

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPERFormat_v1(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPERFormat_v1();

    //
    // Get position of or pointer to data
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////


    //! get Detector buffer length of slot A
    /* cppcheck-suppress missingOverride */
    int Get1stDetectorNwords(int n) OVERRIDE_CPP17;

    //! get Detector buffer length of slot B
    /* cppcheck-suppress missingOverride */
    int Get2ndDetectorNwords(int n) OVERRIDE_CPP17;

    //! get Detector buffer length of slot C
    /* cppcheck-suppress missingOverride */
    int Get3rdDetectorNwords(int n) OVERRIDE_CPP17;

    //! get Detector buffer length of slot D
    /* cppcheck-suppress missingOverride */
    int Get4thDetectorNwords(int n) OVERRIDE_CPP17;

    ///////////////////////////////////////////////////////////////////////////////////////

    //! get buffer pointer of rawcopper trailer
    /* cppcheck-suppress missingOverride */
    int* GetRawTrlBufPtr(int n) OVERRIDE_CPP17;

    //
    // Get information from "RawCOPPERFormat_v1 header" attached by DAQ software
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
    int Get1stFINESSENwords(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE slot B buffer
    /* cppcheck-suppress missingOverride */
    int Get2ndFINESSENwords(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE slot C buffer
    /* cppcheck-suppress missingOverride */
    int Get3rdFINESSENwords(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE slot D buffer
    /* cppcheck-suppress missingOverride */
    int Get4thFINESSENwords(int n) OVERRIDE_CPP17;

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

    /// Format version number
    enum {
      DATA_FORMAT_VERSION = 1
    };

    //! header ( not recorded )
    RawHeader_v1 tmp_header;

    //! trailer ( not recorded )
    RawTrailer_v1 tmp_trailer;


  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(RawCOPPERFormat_v1, 2);
  };




  inline int* RawCOPPERFormat_v1::GetRawTrlBufPtr(int n)
  {
    int pos_nwords;
    if (n == (m_num_events * m_num_nodes) - 1) {
      pos_nwords = m_nwords - tmp_trailer.GetTrlNwords();
    } else {
      pos_nwords = GetBufferPos(n + 1) - tmp_trailer.GetTrlNwords();
    }
    return &(m_buffer[ pos_nwords ]);
  }

  inline int RawCOPPERFormat_v1::GetExpNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpNo();
  }

  inline unsigned int RawCOPPERFormat_v1::GetExpRunSubrun(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpRunSubrun();
  }

  inline int RawCOPPERFormat_v1::GetRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetRunNo();
  }


  inline int RawCOPPERFormat_v1::GetSubRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetSubRunNo();
  }

  inline unsigned int RawCOPPERFormat_v1::GetEveNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetEveNo();
  }


  inline unsigned int RawCOPPERFormat_v1::GetNodeID(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetNodeID();
  }


  inline int RawCOPPERFormat_v1::GetDataType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetDataType();
  }

  inline int RawCOPPERFormat_v1::GetTruncMask(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTruncMask();
  }

  inline unsigned int RawCOPPERFormat_v1::GetErrorBitFlag(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetErrorBitFlag();
  }

  inline void RawCOPPERFormat_v1::AddErrorBitFlag(int n, unsigned int error_bit_flag)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.AddErrorBitFlag(error_bit_flag);
    return;
  }

  inline int RawCOPPERFormat_v1::Get1stDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 0);
  }

  inline int RawCOPPERFormat_v1::Get2ndDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 1);
  }

  inline int RawCOPPERFormat_v1::Get3rdDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 2);
  }

  inline int RawCOPPERFormat_v1::Get4thDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 3);
  }



  inline int RawCOPPERFormat_v1::Get1stFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 0);
  }

  inline int RawCOPPERFormat_v1::Get2ndFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 1);
  }

  inline int RawCOPPERFormat_v1::Get3rdFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 2);
  }

  inline int RawCOPPERFormat_v1::Get4thFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 3);
  }



  inline unsigned int RawCOPPERFormat_v1::GetTTCtimeTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtimeTRGType();
  }

  inline unsigned int RawCOPPERFormat_v1::GetTTUtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTUtime();
  }

  inline int RawCOPPERFormat_v1::GetTTCtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtime();
  }

  inline int RawCOPPERFormat_v1::GetTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTRGType();
  }

  inline void RawCOPPERFormat_v1::GetTTTimeVal(int n, struct timeval* tv)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.GetTTTimeVal(tv);
    return ;
  }

  inline int RawCOPPERFormat_v1::GetMaxNumOfCh(int/* n */)
  {
    return MAX_COPPER_CH;
  }
}
#endif
