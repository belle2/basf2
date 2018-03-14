//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCOPPER_H
#define RAWCOPPER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawDataBlockFormat.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <rawdata/dataobjects/RawCOPPERFormat_latest.h>
#include <rawdata/dataobjects/RawCOPPERFormat_v0.h>
#include <rawdata/dataobjects/RawCOPPERFormat_v1.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_v1.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>
#include <rawdata/RawCOPPERPackerInfo.h>


#include <TObject.h>

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2


// Divide ECL and KLM to barrel and endcap categories from Itoh-san's suggestion
// Updated on May 9, 2014

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// The latest DAQformat version number
#define LATEST_POSTREDUCTION_FORMAT_VER 2 // Since Apr. 21, 2015
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


namespace Belle2 {

  /**
   * The Raw COPPER class
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class RawCOPPER : public RawDataBlock {
  public:
    //! Default constructor
    RawCOPPER();

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPER(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPER();

    //
    // Get position of or pointer to data
    //

    //! set buffer ( delete_flag : m_buffer is freeed( = 0 )/ not freeed( = 1 ) in Destructer )
    void SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes);

    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////
    //! get Detector buffer length
    int GetDetectorNwords(int n, int finesse_num);

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
    int* Get1stDetectorBuffer(int n);

    //! get Detector Buffer of slot B
    int* Get2ndDetectorBuffer(int n);

    //! get Detector Buffer of slot C
    int* Get3rdDetectorBuffer(int n);

    //! get Detector Buffer of slot D
    int* Get4thDetectorBuffer(int n);
    ///////////////////////////////////////////////////////////////////////////////////////

    //! get posistion of COPPER block in unit of word
    virtual int GetBufferPos(int n);

    //! get buffer pointer of rawcopper header(Currently same as GetBufferPos)
    virtual int* GetRawHdrBufPtr(int n);

    //! get buffer pointer of rawcopper trailer
    virtual int* GetRawTrlBufPtr(int n);

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
    // Get information from "RawCOPPER header" attached by DAQ software
    //

    //! get Experimental # from header
    int GetExpNo(int n);    //! get Experimental # from header

    unsigned int GetExpRunSubrun(int n);    //! Exp# (10bit) run# (14bit) restart # (8bit)

    int GetRunNo(int n);    //! get run # (14bit)

    int GetSubRunNo(int n);    //! get subrun #(8bit)

    unsigned int GetEveNo(int n);    //! get contents of header

    int GetDataType(int n);    //! get contents of header

    int GetTruncMask(int n);    //! get contents of header

    //! Get Detected Error bitflag
    unsigned int GetErrorBitFlag(int n);

    //! Get Detected Error bitflag
    void AddErrorBitFlag(int n, unsigned int error_bit_flag);


    //! check CRC packet Error
    int GetPacketCRCError(int n);

    //! check CRC event Error
    int GetEventCRCError(int n);

    //! Get Event CRC16 value
    int GetEventCRC16Value(int n, int finesse_num);

    //! get node-ID from data
    unsigned int GetNodeID(int n);

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    virtual unsigned int GetCOPPERCounter(int n);

    //! get # of FINNESEs which contains data
    virtual int GetNumFINESSEBlock(int n);

    //! get # of offset words
    int GetOffsetFINESSE(int n, int finesse);

    //! get # of offset words for FINESSE slot A buffer position
    int GetOffset1stFINESSE(int n);

    //! get # of offset words for FINESSE slot B buffer position
    int GetOffset2ndFINESSE(int n);

    //! get # of offset words for FINESSE slot C buffer position
    int GetOffset3rdFINESSE(int n);

    //! get # of offset words for FINESSE slot D buffer position
    int GetOffset4thFINESSE(int n);

    //! get data size of  FINESSE buffer
    int GetFINESSENwords(int n, int finesse);

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
    virtual int* GetExpRunSubrunBuf(int n);

    //! get b2l block from "FEE b2link header"
    virtual unsigned int GetB2LFEE32bitEventNumber(int n);

    //
    // read magic word to check data
    //
    //! get magic word of  COPPER driver header
    unsigned int GetMagicDriverHeader(int n);

    //! get magic word of  COPPER FPGA header
    unsigned int GetMagicFPGAHeader(int n);

    //! get magic word of  COPPER FPGA trailer
    unsigned int GetMagicFPGATrailer(int n);

    //! get magic word of  COPPER driver trailer
    unsigned int GetMagicDriverTrailer(int n);

    //! Get checksum in RawTrailer
    unsigned int GetTrailerChksum(int  n);

    //! Check if COPPER Magic words are correct
    bool CheckCOPPERMagic(int n);

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    //! Check B2LFEE header version
    void CheckB2LFEEHeaderVersion(int  n);
#endif

    //! Check if COPPER Magic words are correct
    unsigned int GetTTCtimeTRGType(int n);

    //! Check if COPPER Magic words are correct
    unsigned int GetTTUtime(int n);

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    unsigned int FillTopBlockRawHeader(unsigned int m_node_id,
                                       unsigned int prev_eve32, unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no);

    //! read COPPER driver's checksum value
    unsigned int GetDriverChkSum(int n);

    //! calc COPPER driver's checksum value
    unsigned int CalcDriverChkSum(int n);

    //! calc XOR checksum
    unsigned int CalcXORChecksum(int* buf, int nwords);

    //! check data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no);

    //! check data contents
    void CheckUtimeCtimeTRGType(int n);

    //! Get ctime
    int GetTTCtime(int n);

    //! Get trgtype
    int GetTRGType(int n);

    //! Get timeval
    void GetTTTimeVal(int n, struct timeval* tv);

    //! read data, detect and set the version number of the data format
    void SetVersion();

    //! Get timeval
    void SetVersion(std::string class_name);

    //! Check the version number of data format
    void CheckVersionSetBuffer();

    //! show m_buffer
    void ShowBuffer();

    //! Packer for RawCOPPER class
    //! Pack data (format ver. = -1 -> Select the latest format version)
    void PackDetectorBuf(int* detector_buf_1st, int nwords_1st,
                         int* detector_buf_2nd, int nwords_2nd,
                         int* detector_buf_3rd, int nwords_3rd,
                         int* detector_buf_4th, int nwords_4th,
                         RawCOPPERPackerInfo rawcprpacker_info);

    //! Pack dummy data (format ver. = -1 -> Select the latest format version)
    void PackDetectorBuf4DummyData(int* detector_buf_1st, int nwords_1st,
                                   int* detector_buf_2nd, int nwords_2nd,
                                   int* detector_buf_3rd, int nwords_3rd,
                                   int* detector_buf_4th, int nwords_4th,
                                   RawCOPPERPackerInfo rawcprpacker_info);


    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const;

    enum {
      POS_FORMAT_VERSION = 1,
      FORMAT_MASK = 0x0000FF00
    };

    //! class to access
    RawCOPPERFormat* m_access; //! do not record

    /// Version of the format
    int m_version; //! do not record

  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    ///ver.3 Change FEE format as presented at B2GM in Nov.2013 ( May 1, 2014)
    ///ver.4 Do not record m_access pointer ( Dec 19, 2014)
    ClassDef(RawCOPPER, 3);

  };


  inline int* RawCOPPER::GetRawHdrBufPtr(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetRawHdrBufPtr(n);
  }

  inline int* RawCOPPER::GetRawTrlBufPtr(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetRawTrlBufPtr(n);
  }

  inline unsigned int RawCOPPER::GetDriverChkSum(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetDriverChkSum(n);
  }

  inline int RawCOPPER::GetExpNo(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetExpNo(n);
  }

  inline unsigned int RawCOPPER::GetExpRunSubrun(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetExpRunSubrun(n);
  }

  inline int RawCOPPER::GetRunNo(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetRunNo(n);
  }


  inline int RawCOPPER::GetSubRunNo(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetSubRunNo(n);
  }

  inline unsigned int RawCOPPER::GetEveNo(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetEveNo(n);
  }


  inline unsigned int RawCOPPER::GetNodeID(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetNodeID(n);
  }


  inline int RawCOPPER::GetDataType(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetDataType(n);
  }

  inline int RawCOPPER::GetTruncMask(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTruncMask(n);
  }

  inline unsigned int RawCOPPER::GetErrorBitFlag(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetErrorBitFlag(n);
  }

  inline void RawCOPPER::AddErrorBitFlag(int n, unsigned int error_bit_flag)
  {
    CheckVersionSetBuffer();
    return m_access->AddErrorBitFlag(n, error_bit_flag);
  }

  inline int RawCOPPER::GetPacketCRCError(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetPacketCRCError(n);
  }

  inline int RawCOPPER::GetEventCRCError(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetEventCRCError(n);
  }

  inline int RawCOPPER::GetEventCRC16Value(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetEventCRC16Value(n, finesse_num);
  }

  inline unsigned int RawCOPPER::GetCOPPERCounter(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetCOPPERCounter(n);
  }

  inline int RawCOPPER::Get1stDetectorNwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get1stDetectorNwords(n);
  }

  inline int RawCOPPER::Get2ndDetectorNwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get2ndDetectorNwords(n);
  }

  inline int RawCOPPER::Get3rdDetectorNwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get3rdDetectorNwords(n);
  }

  inline int RawCOPPER::Get4thDetectorNwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get4thDetectorNwords(n);
  }

  inline int RawCOPPER::Get1stFINESSENwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get1stFINESSENwords(n);
  }

  inline int RawCOPPER::Get2ndFINESSENwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get2ndFINESSENwords(n);
  }

  inline int RawCOPPER::Get3rdFINESSENwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get3rdFINESSENwords(n);
  }

  inline int RawCOPPER::Get4thFINESSENwords(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get4thFINESSENwords(n);
  }

  inline int RawCOPPER::GetOffset1stFINESSE(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetOffset1stFINESSE(n);
  }

  inline int RawCOPPER::GetOffset2ndFINESSE(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetOffset2ndFINESSE(n);
  }

  inline int RawCOPPER::GetOffset3rdFINESSE(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetOffset3rdFINESSE(n);
  }

  inline int RawCOPPER::GetOffset4thFINESSE(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetOffset4thFINESSE(n);
  }

  inline int* RawCOPPER::Get1stFINESSEBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get1stFINESSEBuffer(n);
  }

  inline int* RawCOPPER::Get2ndFINESSEBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get2ndFINESSEBuffer(n);
  }

  inline int* RawCOPPER::Get3rdFINESSEBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get3rdFINESSEBuffer(n);
  }

  inline int* RawCOPPER::Get4thFINESSEBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get4thFINESSEBuffer(n);
  }

  inline int* RawCOPPER::Get1stDetectorBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get1stDetectorBuffer(n);
  }

  inline int* RawCOPPER::Get2ndDetectorBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get2ndDetectorBuffer(n);
  }

  inline int* RawCOPPER::Get3rdDetectorBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get3rdDetectorBuffer(n);
  }

  inline int* RawCOPPER::Get4thDetectorBuffer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->Get4thDetectorBuffer(n);
  }

  inline int* RawCOPPER::GetExpRunSubrunBuf(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetExpRunSubrunBuf(n);
  }

  inline unsigned int RawCOPPER::GetMagicDriverHeader(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetMagicDriverHeader(n);
  }

  inline unsigned int RawCOPPER::GetMagicFPGAHeader(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetMagicFPGAHeader(n);
  }

  inline unsigned int RawCOPPER::GetMagicFPGATrailer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetMagicFPGATrailer(n);
  }

  inline unsigned int RawCOPPER::GetMagicDriverTrailer(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetMagicDriverTrailer(n);
  }

  inline unsigned int RawCOPPER::GetTrailerChksum(int  n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTrailerChksum(n);
  }

  inline unsigned int RawCOPPER::GetTTCtimeTRGType(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTTCtimeTRGType(n);
  }

  inline unsigned int RawCOPPER::GetTTUtime(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTTUtime(n);
  }

  inline int RawCOPPER::GetTTCtime(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTTCtime(n);
  }

  inline int RawCOPPER::GetTRGType(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetTRGType(n);
  }

  inline void RawCOPPER::GetTTTimeVal(int n, struct timeval* tv)
  {
    CheckVersionSetBuffer();
    return m_access->GetTTTimeVal(n, tv);
  }

  inline int RawCOPPER::GetBufferPos(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetBufferPos(n);
  }

  inline   unsigned int RawCOPPER::CalcDriverChkSum(int n)
  {
    CheckVersionSetBuffer();
    return m_access->CalcDriverChkSum(n);
  }


  inline  int RawCOPPER::GetNumFINESSEBlock(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetNumFINESSEBlock(n);
  }

  inline  int RawCOPPER::GetDetectorNwords(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetDetectorNwords(n, finesse_num);
  }

  inline  int RawCOPPER::GetFINESSENwords(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetFINESSENwords(n, finesse_num);
  }

  inline  int RawCOPPER::GetOffsetFINESSE(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetOffsetFINESSE(n, finesse_num);
  }

  inline  int* RawCOPPER::GetFINESSEBuffer(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetFINESSEBuffer(n, finesse_num);
  }

  inline  int* RawCOPPER::GetDetectorBuffer(int n, int finesse_num)
  {
    CheckVersionSetBuffer();
    return m_access->GetDetectorBuffer(n, finesse_num);
  }

  inline  unsigned int RawCOPPER::GetB2LFEE32bitEventNumber(int n)
  {
    CheckVersionSetBuffer();
    return m_access->GetB2LFEE32bitEventNumber(n);
  }

  inline  unsigned int RawCOPPER::CalcXORChecksum(int* buf, int nwords)
  {
    CheckVersionSetBuffer();
    return m_access->CalcXORChecksum(buf, nwords);
  }

  inline  void RawCOPPER::CheckData(int n,
                                    unsigned int prev_evenum, unsigned int* cur_evenum_rawcprhdr,
                                    unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                                    unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
  {

    CheckVersionSetBuffer();
    m_access->CheckData(n,
                        prev_evenum, cur_evenum_rawcprhdr,
                        prev_copper_ctr, cur_copper_ctr,
                        prev_exprunsubrun_no,  cur_exprunsubrun_no);
    return;
  }

  inline  bool RawCOPPER::CheckCOPPERMagic(int n)
  {
    CheckVersionSetBuffer();
    return m_access->CheckCOPPERMagic(n);
  }

  inline  void RawCOPPER::CheckUtimeCtimeTRGType(int n)
  {
    CheckVersionSetBuffer();
    return m_access->CheckUtimeCtimeTRGType(n);
  }

  inline  unsigned int RawCOPPER::GetB2LHeaderWord(int n, int finesse_buffer_pos)
  {
    CheckVersionSetBuffer();
    return m_access->GetB2LHeaderWord(n, finesse_buffer_pos);
  }

  inline  unsigned int RawCOPPER::FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
                                                        unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
  {
    CheckVersionSetBuffer();
    return m_access->FillTopBlockRawHeader(m_node_id, prev_eve32,
                                           prev_exprunsubrun_no, cur_exprunsubrun_no);
  }

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
  inline  void RawCOPPER::CheckB2LFEEHeaderVersion(int n)
  {
    CheckVersionSetBuffer();
    return m_access->CheckB2LFEEHeaderVersion(n);
  }
#endif

  inline void RawCOPPER::CheckVersionSetBuffer()
  {
    if (((m_buffer[ POS_FORMAT_VERSION ] & FORMAT_MASK) >> 8)  != m_version
        || m_access == NULL) {
      SetVersion();
    }
    m_access->SetBuffer(m_buffer, m_nwords, 0, m_num_events, m_num_nodes);
  }
}

#endif
