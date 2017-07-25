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
#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <rawdata/RawCOPPERPackerInfo.h>
#include <rawdata/dataobjects/RawDataBlockFormat.h>

/* #include <framework/datastore/DataStore.h> */
/* #include <TObject.h> */

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

#define DETECTOR_MASK 0xFF000000 // tentative
#define COPPERID_MASK 0x00FFFFFF // tentative
#define SVD_ID  0x01000000 // tentative
#define CDC_ID  0x02000000 // tentative
#define TOP_ID 0x03000000 // tentative
#define ARICH_ID 0x04000000 // tentative
#define BECL_ID  0x05000000 // tentative
#define EECL_ID  0x06000000 // tentative
#define BKLM_ID  0x07000000 // tentative
#define EKLM_ID  0x08000000 // tentative
#define TRGDATA_ID  0x10000000 // tentative
#define CDCTRGDATA_ID  0x11000000 // tentative
#define TOPTRGDATA_ID  0x12000000 // tentative
#define ECLTRGDATA_ID  0x13000000 // tentative
#define KLMTRGDATA_ID  0x14000000 // tentative
#define GDLTRGDATA_ID  0x15000000 // tentative


namespace Belle2 {

  /**
   * The Raw COPPER class
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  //  class RawCOPPERFormat : public TObject {
  class RawCOPPERFormat : public RawDataBlockFormat {
  public:
    /*     //! Default constructor */
    RawCOPPERFormat();

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPERFormat(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPERFormat() {}

    //
    // Functions for  RawDataBlock
    //
    //! set buffer ( delete_flag : m_buffer is freeed( = 0 )/ not freeed( = 1 ) in Destructer )
    /*     void SetBuffer(int* bufin, int nwords, int delete_flag, int num_events, int num_nodes); */

    /*     //! Get total length of m_buffer */
    /*     virtual int TotalBufNwords(); */

    /*     //! get nth buffer pointer */
    /*     virtual int* GetBuffer(int n); */

    /*     //! get pointer to  buffer(m_buffer) */
    /*     virtual int* GetWholeBuffer(); */

    /*     //! get # of data blocks = (# of nodes)*(# of events) */
    /*     virtual int GetNumEntries() { return m_num_events * m_num_nodes; } */

    /*     //! get # of data sources(e.g. # of COPPER boards) in m_buffer */
    /*     virtual int GetNumNodes() { return m_num_nodes; } */

    /*     //! get # of events in m_buffer */
    /*     virtual int GetNumEvents() { return m_num_events; } */

    /*     //! get size of a data block */
    /*     virtual int GetBlockNwords(int n); */

    /*      //! print out data  */
    /*     virtual void PrintData(int* buf, int nwords); */

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

    //
    // Functions for RawCOPPER
    //

    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR

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
    virtual int* GetDetectorBuffer(int n, int finesse_num);

    //! get Detector buffer of slot A
    virtual int* Get1stDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot B
    virtual int* Get2ndDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot C
    virtual int* Get3rdDetectorBuffer(int n) = 0;

    //! get Detector Buffer of slot D
    virtual int* Get4thDetectorBuffer(int n) = 0;
    ///////////////////////////////////////////////////////////////////////////////////////

    /*     //! get posistion of COPPER block in unit of word */
    /*     virtual int GetBufferPos(int n) = 0; */

    //! get buffer pointer of rawcopper header(Currently same as GetBufferPos)
    virtual int* GetRawHdrBufPtr(int n);

    //! get buffer pointer of rawcopper trailer
    virtual int* GetRawTrlBufPtr(int n) = 0;

    //! get FINESSE buffer pointer
    virtual int* GetFINESSEBuffer(int n, int finesse_num);

    //! get FINESSE buffer pointer for slot A
    virtual int* Get1stFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot B
    virtual int* Get2ndFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot C
    virtual int* Get3rdFINESSEBuffer(int n);

    //! get FINESSE buffer pointer for slot D
    virtual int* Get4thFINESSEBuffer(int n);



    //
    // Get information from "RawCOPPERFormat header" attached by DAQ software
    //

    /// get Experimental # from header
    virtual int GetExpNo(int n) = 0;    //! get Experimental # from header

    virtual unsigned int GetExpRunSubrun(int n) = 0;    //! Exp# (10bit) run# (14bit) restart # (8bit)

    virtual int GetRunNo(int n) = 0;    //! get run # (14bit)

    virtual int GetSubRunNo(int n) = 0;    //! get subrun #(8bit)

    virtual unsigned int GetEveNo(int n) = 0;    //! get contents of header

    virtual int GetDataType(int n) = 0;    //! get contents of header

    virtual int GetTruncMask(int n) = 0;    //! get contents of header

    //! Get Detected Error bitflag
    virtual unsigned int GetErrorBitFlag(int n) = 0;

    //! Add Detected Error bitflag
    virtual void AddErrorBitFlag(int n, unsigned int error_bit_flag) = 0;

    //! check CRC packet Error
    virtual int GetPacketCRCError(int n);

    //! check CRC event Error
    virtual int GetEventCRCError(int n);

    //! Get Event CRC16 value
    virtual int GetEventCRC16Value(int n, int finesse_num);

    //! get node-ID from data
    virtual unsigned int GetNodeID(int n) = 0;

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    virtual unsigned int GetCOPPERCounter(int n) = 0;

    //! get # of FINNESEs which contains data
    virtual int GetNumFINESSEBlock(int n) = 0;

    //! get # of offset words
    virtual int GetOffsetFINESSE(int n, int finesse);

    //! get # of offset words for FINESSE slot A buffer position
    virtual int GetOffset1stFINESSE(int n) = 0;

    //! get # of offset words for FINESSE slot B buffer position
    virtual int GetOffset2ndFINESSE(int n);

    //! get # of offset words for FINESSE slot C buffer position
    virtual int GetOffset3rdFINESSE(int n);

    //! get # of offset words for FINESSE slot D buffer position
    virtual int GetOffset4thFINESSE(int n);

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
    virtual unsigned int GetB2LHeaderWord(int n, int finesse_buffer_pos);

    //
    // Get information from "B2link(attached by FEE and HLSB) header"
    //
    //! get b2l block from "FEE b2link header"
    //    virtual int* GetFTSW2Words(int n) = 0;

    //! get b2l block from "FEE b2link header"
    virtual int* GetExpRunSubrunBuf(int n) = 0;

    //! get b2l block from "FEE b2link header"
    virtual unsigned int GetB2LFEE32bitEventNumber(int n) = 0;

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
    virtual unsigned int FillTopBlockRawHeader(unsigned int m_node_id,
                                               unsigned int prev_eve32, unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) = 0;

    //! read COPPER driver's checksum value
    virtual unsigned int GetDriverChkSum(int n) = 0;

    //! calc COPPER driver's checksum value
    virtual unsigned int CalcDriverChkSum(int n) = 0;


    //! check data contents
    virtual void CheckData(int n,
                           unsigned int prev_evenum, unsigned int* cur_evenum,
                           unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                           unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) = 0;

    //! check data contents
    virtual void CheckUtimeCtimeTRGType(int n) = 0;

    //! Get ctime
    virtual int GetTTCtime(int n) = 0;

    //! Get trgtype
    virtual int GetTRGType(int n) = 0;

    //! Get timeval
    virtual void GetTTTimeVal(int n, struct timeval* tv) = 0;

    //! calc XOR checksum
    virtual unsigned int CalcXORChecksum(int* buf, int nwords);

    //! Pack data (format ver. = -1 -> Select the latest format version)
    virtual int* PackDetectorBuf(int* packed_buf_nwords,
                                 int* detector_buf_1st, int nwords_1st,
                                 int* detector_buf_2nd, int nwords_2nd,
                                 int* detector_buf_3rd, int nwords_3rd,
                                 int* detector_buf_4th, int nwords_4th,
                                 RawCOPPERPackerInfo rawcprpacker_info) = 0;

  protected :

    /*     /// number of words of buffer */
    /*     int m_nwords; */

    /*     /// number of nodes in this object */
    /*     int m_num_nodes; */

    /*     /// number of events in this object */
    /*     int m_num_events; */

    /*     /// Buffer to access data */
    /*     int* m_buffer; //! not recorded */

    ///ver.3 : Separate from RawDataBLock to avoid a memory leak ( July 24, 2014)
    //    ClassDef(RawCOPPERFormat, 3);

  };



  inline int* RawCOPPERFormat::GetRawHdrBufPtr(int n)
  {
    int pos_nwords = GetBufferPos(n);
    return &(m_buffer[ pos_nwords ]);
  }


  inline int RawCOPPERFormat::GetOffset2ndFINESSE(int n)
  {
    return GetOffset1stFINESSE(n) + Get1stFINESSENwords(n);
  }

  inline int RawCOPPERFormat::GetOffset3rdFINESSE(int n)
  {
    return GetOffset2ndFINESSE(n) + Get2ndFINESSENwords(n);
  }

  inline int RawCOPPERFormat::GetOffset4thFINESSE(int n)
  {
    return GetOffset3rdFINESSE(n) + Get3rdFINESSENwords(n);
  }



  inline int* RawCOPPERFormat::Get1stFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset1stFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size(0x%.8x) is smaller than data position info(0x%.8x). Exting...\n",  m_nwords, pos_nwords);
      for (int i = 0; i < m_nwords; i++) {
        printf("%.8x ", m_buffer[ i ]);
        if ((i % 10) == 9) printf("\n");
      }
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat::Get2ndFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset2ndFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size(0x%.8x) is smaller than data position info(0x%.8x). Exting...\n",  m_nwords, pos_nwords);
      for (int i = 0; i < m_nwords; i++) {
        printf("%.8x ", m_buffer[ i ]);
        if ((i % 10) == 9) printf("\n");
      }
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat::Get3rdFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset3rdFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size(0x%.8x) is smaller than data position info(0x%.8x). Exting...\n",  m_nwords, pos_nwords);
      for (int i = 0; i < m_nwords; i++) {
        printf("%.8x ", m_buffer[ i ]);
        if ((i % 10) == 9) printf("\n");
      }
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* RawCOPPERFormat::Get4thFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset4thFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size(0x%.8x) is smaller than data position info(0x%.8x). Exting...\n",  m_nwords, pos_nwords);
      for (int i = 0; i < m_nwords; i++) {
        printf("%.8x ", m_buffer[ i ]);
        if ((i % 10) == 9) printf("\n");
      }
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }


}

#endif
