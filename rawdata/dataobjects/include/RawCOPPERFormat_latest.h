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



#define DETECTOR_MASK 0xFF000000 // tentative
#define SVD_ID  0x01000000 // tentative
#define CDC_ID  0x02000000 // tentative
#define BPID_ID 0x03000000 // tentative
#define EPID_ID 0x04000000 // tentative
#define ECL_ID  0x05000000 // tentative
#define KLM_ID  0x06000000 // tentative


namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

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


    //! get Detector buffer length
    virtual int GetDetectorNwords(int n, int finesse_num) = 0;

    //! get Detector buffer length of slot A
    int Get1stDetectorNwords(int n);

    //! get Detector buffer length of slot B
    int Get2ndDetectorNwords(int n);

    //! get Detector buffer length of slot C
    int Get3rdDetectorNwords(int n);

    //! get Detector buffer length of slot D
    int Get4thDetectorNwords(int n);

    //! get Detector buffer
    int* GetDetectorBuffer(int n, int finesse_num);

    //! get Detector buffer of slot A
    virtual int* Get1stDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot B
    virtual int* Get2ndDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot C
    virtual int* Get3rdDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot D
    virtual int* Get4thDetectorBuffer(int n) = 0;

    ///////////////////////////////////////////////////////////////////////////////////////

    //! get posistion of COPPER block in unit of word
    virtual int GetBufferPos(int n) = 0;

    //! get buffer pointer of rawcopper header(Currently same as GetBufferPos)
    int* GetRawHdrBufPtr(int n);

    //! get buffer pointer of rawcopper trailer
    int* GetRawTrlBufPtr(int n);

    //! get FINESSE buffer pointer
    int* GetFINESSEBuffer(int n, int finesse_num);

    //! get FINESSE buffer pointer for slot A
    int* Get1stFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot B
    int* Get2ndFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot C
    int* Get3rdFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot D
    int* Get4thFINESSEBuffer(int n);


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
    //! get COPPER counter(not event number)
    virtual unsigned int GetCOPPERCounter(int n) = 0;

    //! get COPPER node id from data(Currently same as GetCOPPERNodeId)
    virtual int GetCOPPERNodeId(int n);

    //! get # of FINNESEs which contains data
    virtual int GetNumFINESSEBlock(int n);

    //! get # of offset words
    int GetOffsetFINESSE(int n, int finesse);

    //! get # of offset words for FINESSE slot A buffer position
    virtual int GetOffset1stFINESSE(int n) = 0;

    //! get # of offset words for FINESSE slot B buffer position
    int GetOffset2ndFINESSE(int n);

    //! get # of offset words for FINESSE slot C buffer position
    int GetOffset3rdFINESSE(int n);

    //! get # of offset words for FINESSE slot D buffer position
    int GetOffset4thFINESSE(int n);

    //! get data size of  FINESSE buffer
    virtual int GetFINESSENwords(int n, int finesse) = 0;

    //! get data size of  FINESSE slot A buffer
    int Get1stFINESSENwords(int n);

    //! get data size of  FINESSE slot B buffer
    int Get2ndFINESSENwords(int n);

    //! get data size of  FINESSE slot C buffer
    int Get3rdFINESSENwords(int n);

    //! get data size of  FINESSE slot D buffer
    int Get4thFINESSENwords(int n);

    //! For copying FTSW word1 (FEE header)
    unsigned int GetB2LHeaderWord(int n, int finesse_buffer_pos);

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    //    virtual int* GetFTSW2Words(int n);

    //! get b2l block from "FEE b2link header"
    virtual int* GetExpRunBuf(int n) = 0;

    //! get b2l block from "FEE b2link header"
    virtual unsigned int GetB2LFEE32bitEventNumber(int n) = 0;

    //! get Event unixtime from "FEE b2link header"
    virtual double GetEventUnixTime(int n) = 0;

    //
    // read magic word to check data
    //
    //! get magic word of  COPPER driver header
    virtual unsigned int GetMagicDriverHeader(int n) = 0;

    //! get magic word of  COPPER FPGA header
    virtual unsigned int GetMagicFPGAHeader(int n) = 0;

    //! get magic word of  COPPER FPGA trailer
    virtual unsigned int GetMagicFPGATrailer(int n) = 0;

    //! get magic word of  COPPER driver trailer
    virtual unsigned int GetMagicDriverTrailer(int n) = 0;

    //! get a checksum on trailer
    virtual unsigned int GetTrailerChksum(int  n) = 0;

    //! Check if COPPER Magic words are correct
    virtual bool CheckCOPPERMagic(int n) = 0;

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    //! Check B2LFEE header version
    void CheckB2LFEEHeaderVersion(int  n);
#endif

    //! Check if COPPER Magic words are correct
    unsigned int GetTTCtimeTRGType(int n);

    //! Check if COPPER Magic words are correct
    unsigned int GetTTUtime(int n);

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    virtual unsigned int FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type,
                                               unsigned int m_trunc_mask, unsigned int prev_eve32,
                                               int prev_run_no, int* cur_run_no) = 0;

    //! read COPPER driver's checksum value
    virtual unsigned int GetDriverChkSum(int n) = 0;

    //! calc COPPER driver's checksum value
    virtual unsigned int CalcDriverChkSum(int n) = 0;

    //! calc XOR checksum
    unsigned int CalcXORChecksum(int* buf, int nwords);

    //! check data contents
    virtual void CheckData(int n,
                           unsigned int prev_evenum, unsigned int* cur_evenum,
                           unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                           int prev_run_no, int* cur_run_no) = 0;

    //! check data contents
    void CheckUtimeCtimeTRGType(int n) = 0;

    //! Get ctime
    int GetTTCtime(int n);

    //! Get timeval
    void GetTTTimeVal(int n, struct timeval* tv);

    //! check magic words of B2link HSLB header/trailer
    virtual int CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords) = 0;

    //! header ( not recorded )
    RawHeader_latest tmp_header;

    //! trailer ( not recorded )
    RawTrailer_latest tmp_trailer;

  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    ClassDef(RawCOPPERFormat_latest, 2);

  };



  inline int* RawCOPPERFormat_latest::GetRawHdrBufPtr(int n)
  {
    int pos_nwords = GetBufferPos(n);
    return &(m_buffer[ pos_nwords ]);
  }

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

  inline int RawCOPPERFormat_latest::GetOffset2ndFINESSE(int n)
  {
    return GetOffset1stFINESSE(n) + Get1stFINESSENwords(n);
  }

  inline int RawCOPPERFormat_latest::GetOffset3rdFINESSE(int n)
  {
    return GetOffset2ndFINESSE(n) + Get2ndFINESSENwords(n);
  }

  inline int RawCOPPERFormat_latest::GetOffset4thFINESSE(int n)
  {
    return GetOffset3rdFINESSE(n) + Get3rdFINESSENwords(n);
  }


  inline int* RawCOPPERFormat_latest::Get1stFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset1stFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat_latest::Get2ndFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset2ndFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat_latest::Get3rdFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset3rdFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat_latest::Get4thFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset4thFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }




  inline unsigned int RawCOPPERFormat_latest::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), trl.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_latest::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_latest::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 2;
    return (unsigned int)(m_buffer[ pos_nwords ]);
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
