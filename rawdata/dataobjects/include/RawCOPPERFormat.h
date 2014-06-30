//+
// File : RawCOPPERFormat.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#ifndef RAWCOPPERFORMAT_H
#define RAWCOPPERFORMAT_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <framework/datastore/DataStore.h>


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
   * The Raw COPPER class
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class RawCOPPERFormat : public RawDataBlock {
  public:
    /*     //! Default constructor */
    RawCOPPERFormat() {}

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPERFormat(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPERFormat() {}

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
    virtual int Get1stDetectorNwords(int n) = 0;

    //! get Detector buffer length of slot B
    virtual int Get2ndDetectorNwords(int n) = 0;

    //! get Detector buffer length of slot C
    virtual int Get3rdDetectorNwords(int n) = 0;

    //! get Detector buffer length of slot D
    virtual int Get4thDetectorNwords(int n) = 0;

    //! get Detector buffer
    virtual int* GetDetectorBuffer(int n, int finesse_num) = 0;

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
    virtual int* GetRawHdrBufPtr(int n) = 0;

    //! get buffer pointer of rawcopper trailer
    virtual int* GetRawTrlBufPtr(int n) = 0;

    //! get FINESSE buffer pointer
    virtual int* GetFINESSEBuffer(int n, int finesse_num) = 0;

    //! get FINESSE buffer pointer for slot A
    virtual int* Get1stFINESSEBuffer(int n) = 0;

    //! get FINESSE buffer pointer for slot B
    virtual int* Get2ndFINESSEBuffer(int n) = 0;

    //! get FINESSE buffer pointer for slot C
    virtual int* Get3rdFINESSEBuffer(int n) = 0;

    //! get FINESSE buffer pointer for slot D
    virtual int* Get4thFINESSEBuffer(int n) = 0;


    //
    // Get information from "RawCOPPERFormat header" attached by DAQ software
    //
    virtual int GetExpNo(int n) = 0;    //! get Experimental # from header

    virtual int GetRunNoSubRunNo(int n) = 0;    //! run# (14bit) restart # (8bit)

    virtual int GetRunNo(int n) = 0;    //! get run # (14bit)

    virtual int GetSubRunNo(int n) = 0;    //! get subrun #(8bit)

    virtual unsigned int GetEveNo(int n) = 0;    //! get contents of header

    virtual int GetDataType(int n) = 0;    //! get contents of header

    virtual int GetTruncMask(int n) = 0;    //! get contents of header

    virtual int GetSubsysId(int n) = 0;     //! get subsystem-ID from data

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    virtual unsigned int GetCOPPERCounter(int n) = 0;

    //! get COPPER node id from data(Currently same as GetCOPPERNodeId)
    virtual int GetCOPPERNodeId(int n) = 0;

    //! get # of FINNESEs which contains data
    virtual int GetNumFINESSEBlock(int n) = 0;

    //! get # of offset words
    virtual int GetOffsetFINESSE(int n, int finesse) = 0;

    //! get # of offset words for FINESSE slot A buffer position
    virtual int GetOffset1stFINESSE(int n) = 0;

    //! get # of offset words for FINESSE slot B buffer position
    virtual int GetOffset2ndFINESSE(int n) = 0;

    //! get # of offset words for FINESSE slot C buffer position
    virtual int GetOffset3rdFINESSE(int n) = 0;

    //! get # of offset words for FINESSE slot D buffer position
    virtual int GetOffset4thFINESSE(int n) = 0;

    //! get data size of  FINESSE buffer
    virtual int GetFINESSENwords(int n, int finesse) = 0;

    //! get data size of  FINESSE slot A buffer
    virtual int Get1stFINESSENwords(int n) = 0;

    //! get data size of  FINESSE slot B buffer
    virtual int Get2ndFINESSENwords(int n) = 0;

    //! get data size of  FINESSE slot C buffer
    virtual int Get3rdFINESSENwords(int n) = 0;

    //! get data size of  FINESSE slot D buffer
    virtual int Get4thFINESSENwords(int n) = 0;

    //! For copying FTSW word1 (FEE header)
    virtual unsigned int GetB2LHeaderWord(int n, int finesse_buffer_pos) = 0;

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    //    virtual int* GetFTSW2Words(int n) = 0;

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

    //! Get checksum in RawTrailer
    virtual unsigned int GetTrailerChksum(int  n) = 0 ;

    //! Check if COPPER Magic words are correct
    virtual bool CheckCOPPERMagic(int n) = 0;

#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    //! Check B2LFEE header version
    virtual void CheckB2LFEEHeaderVersion(int  n) = 0;
#endif

    //! Check if COPPER Magic words are correct
    virtual unsigned int GetTTCtimeTRGType(int n) = 0;

    //! Check if COPPER Magic words are correct
    virtual unsigned int GetTTUtime(int n) = 0;

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    virtual unsigned int FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type, unsigned int m_trunc_mask,
                                               unsigned int prev_eve32, int prev_run_no, int* cur_run_no) = 0;

    //! read COPPER driver's checksum value
    virtual unsigned int GetDriverChkSum(int n) = 0;

    //! calc COPPER driver's checksum value
    virtual unsigned int CalcDriverChkSum(int n) = 0;


    //! check data contents
    virtual void CheckData(int n,
                           unsigned int prev_evenum, unsigned int* cur_evenum,
                           unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                           int prev_run_no, int* cur_run_no) = 0;

    //! check data contents
    virtual void CheckUtimeCtimeTRGType(int n) = 0;

    //! Get ctime
    virtual int GetTTCtime(int n) = 0;

    //! Get timeval
    virtual void GetTTTimeVal(int n, struct timeval* tv) = 0;

    //! calc XOR checksum
    virtual unsigned int CalcXORChecksum(int* buf, int nwords) = 0;


  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    ClassDef(RawCOPPERFormat, 2);

  };


}

#endif
