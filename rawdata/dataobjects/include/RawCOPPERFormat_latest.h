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
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/datastore/DataStore.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <rawdata/dataobjects/RawHeader_latest.h>
#include <rawdata/dataobjects/RawTrailer_latest.h>
#include <rawdata/CRCCalculator.h>

#include <TObject.h>

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
    int Get1stDetectorNwords(int n);

    //! get Detector buffer length of slot B
    int Get2ndDetectorNwords(int n);

    //! get Detector buffer length of slot C
    int Get3rdDetectorNwords(int n);

    //! get Detector buffer length of slot D
    int Get4thDetectorNwords(int n);

    ///////////////////////////////////////////////////////////////////////////////////////

    //! get buffer pointer of rawcopper trailer
    int* GetRawTrlBufPtr(int n);

    //
    // Get information from "RawCOPPERFormat_latest header" attached by DAQ software
    //

    int GetExpNo(int n);    //! get Experimental # from header

    int GetRunNoSubRunNo(int n);    //! run# (14bit) restart # (8bit)

    int GetRunNo(int n);    //! get run # (14bit)

    int GetSubRunNo(int n);    //! get subrun #(8bit)

    unsigned int GetEveNo(int n);    //! get contents of header

    int GetDataType(int n);    //! get contents of header

    int GetTruncMask(int n);    //! get contents of header

    int GetSubsysId(int n);     //! get subsystem-ID from data

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //

    //! get COPPER node id from data(Currently same as GetCOPPERNodeId)
    virtual int GetCOPPERNodeId(int n);

    //! get # of FINNESEs which contains data
    virtual int GetNumFINESSEBlock(int n);

    //! get data size of  FINESSE slot A buffer
    int Get1stFINESSENwords(int n);

    //! get data size of  FINESSE slot B buffer
    int Get2ndFINESSENwords(int n);

    //! get data size of  FINESSE slot C buffer
    int Get3rdFINESSENwords(int n);

    //! get data size of  FINESSE slot D buffer
    int Get4thFINESSENwords(int n);

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    //    virtual int* GetFTSW2Words(int n);

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    //! Check B2LFEE header version
    void CheckB2LFEEHeaderVersion(int  n);
#endif

    //! Check if COPPER Magic words are correct
    unsigned int GetTTCtimeTRGType(int n);

    //! Check if COPPER Magic words are correct
    unsigned int GetTTUtime(int n);

    /*     //! calc COPPER driver's checksum value */
    /*     virtual unsigned int CalcDriverChkSum(int n) = 0; */


    //! Get ctime
    int GetTTCtime(int n);

    //! Get timeval
    void GetTTTimeVal(int n, struct timeval* tv);


    //! header ( not recorded )
    RawHeader_latest tmp_header;

    //! trailer ( not recorded )
    RawTrailer_latest tmp_trailer;


  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    ClassDef(RawCOPPERFormat_latest, 2);
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


  inline int RawCOPPERFormat_latest::GetCOPPERNodeId(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.POS_SUBSYS_ID;
    return m_buffer[ pos_nwords ];
  }


  inline int RawCOPPERFormat_latest::GetExpNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpNo();
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

  inline int RawCOPPERFormat_latest::GetRunNoSubRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetRunNoSubRunNo();
  }

  inline unsigned int RawCOPPERFormat_latest::GetEveNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetEveNo();
  }


  inline int RawCOPPERFormat_latest::GetSubsysId(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetSubsysId();
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


  inline int RawCOPPERFormat_latest::Get1stDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 0);
  }

  inline int RawCOPPERFormat_latest::Get2ndDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 1);
  }

  inline int RawCOPPERFormat_latest::Get3rdDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 2);
  }

  inline int RawCOPPERFormat_latest::Get4thDetectorNwords(int n)
  {
    return GetDetectorNwords(n, 3);
  }



  inline int RawCOPPERFormat_latest::Get1stFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 0);
  }

  inline int RawCOPPERFormat_latest::Get2ndFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 1);
  }

  inline int RawCOPPERFormat_latest::Get3rdFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 2);
  }

  inline int RawCOPPERFormat_latest::Get4thFINESSENwords(int n)
  {
    return GetFINESSENwords(n, 3);
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

  inline void RawCOPPERFormat_latest::GetTTTimeVal(int n, struct timeval* tv)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.GetTTTimeVal(tv);
    return ;
  }
}
#endif
