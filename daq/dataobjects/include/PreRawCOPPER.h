//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRERAWCOPPER_H
#define PRERAWCOPPER_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/CRCCalculator.h>
#include <daq/dataobjects/ReducedRawCOPPER.h>
#include <daq/dataobjects/ReducedRawHeader.h>
#include <daq/dataobjects/ReducedRawTrailer.h>
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
  class PreRawCOPPER : public RawDataBlock {
  public:
    //! Default constructor
    PreRawCOPPER();

    //! Constructor using existing pointer to raw data buffer
    //PreRawCOPPER(int* bufin, int nwords);
    //! Destructor
    virtual ~PreRawCOPPER();

    //
    // Get position of or pointer to data
    //
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
    // Get information from "PreRawCOPPER header" attached by DAQ software
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
    virtual unsigned int GetCOPPERCounter(int n);

    //! get COPPER node id from data(Currently same as GetCOPPERNodeId)
    virtual int GetCOPPERNodeId(int n);

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
    virtual int* GetExpRunBuf(int n);

    //! get b2l block from "FEE b2link header"
    virtual unsigned int GetB2LFEE32bitEventNumber(int n);

    //! get Event unixtime from "FEE b2link header"
    double GetEventUnixTime(int n);

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
    unsigned int FillTopBlockRawHeader(unsigned int m_node_id, unsigned int m_data_type, unsigned int m_trunc_mask,
                                       unsigned int prev_eve32, int prev_run_no, int* cur_run_no);

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
                   int prev_run_no, int* cur_run_no);

    //! check data contents
    void CheckUtimeCtimeTRGType(int n);

    //! Get ctime
    int GetTTCtime(int n);

    //! Get timeval
    void GetTTTimeVal(int n, struct timeval* tv);

    //
    // Reduce data size
    //

    //!
    int CalcReducedNwords(int n);

    //!
    int CopyReducedBuffer(int n, int* buf_to);

    //!
    int CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords);


    //
    // size of "COPPER front header" and "COPPER trailer"
    //
    //! Copper data words = ( total_data_length in COPPER header ) + COPPER_HEADER_TRAILER_NWORDS
    enum {
      SIZE_COPPER_DRIVER_HEADER = 7,
      SIZE_COPPER_DRIVER_TRAILER = 2
    };

    //
    // Data Format : "COPPER header"
    //
    enum {
      POS_MAGIC_COPPER_1 = 0,
      POS_EVE_NUM_COPPER = 1,
      POS_SUBSYSTEM_ID = 2,
      POS_CRATE_ID = 3,
      POS_SLOT_ID = 4,
      POS_MAGIC_COPPER_2 = 7,
      POS_DATA_LENGTH = 8,
      POS_CH_A_DATA_LENGTH = 9,
      POS_CH_B_DATA_LENGTH = 10,
      POS_CH_C_DATA_LENGTH = 11,
      POS_CH_D_DATA_LENGTH = 12,

      SIZE_COPPER_HEADER = 13
    };



    //
    // Data Format : "COPPER Trailer"
    //
    enum {
      POS_MAGIC_COPPER_3 = 0,
      POS_CHKSUM_COPPER = 1,
      POS_MAGIC_COPPER_4 = 2,

      SIZE_COPPER_TRAILER = 3
    };

    //
    // Data Format : "B2Link HSLB Header"
    //
    enum {
      POS_MAGIC_B2LHSLB = 0,
      //      POS_EVE_CNT_B2LHSLB = 1,
      SIZE_B2LHSLB_HEADER = 1
    };


    //
    // Data Format : "B2Link HSLB Trailer"
    //
    enum {
      POS_CHKSUM_B2LHSLB = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };

    enum {
      B2LHSLB_HEADER_MAGIC = 0xFFAA0000,
      B2LHSLB_TRAILER_MAGIC = 0xFF550000
    };


    // Data Format : "B2Link FEE Header"
    // modified by Nov. 21, 2013, Nakao-san's New firmware?
    enum {
      POS_TT_CTIME_TYPE = 0,
      POS_TT_TAG = 1,
      POS_TT_UTIME = 2,
      POS_EXP_RUN = 3,
      POS_B2L_CTIME = 4,
      SIZE_B2LFEE_HEADER = 5
    };


    //
    // Data Format : B2Link FEE Trailer
    //
    enum {
      POS_CHKSUM_B2LFEE = 0,
      SIZE_B2LFEE_TRAILER = 1
    };

    //
    // COPPER magic words
    //
    enum {
      COPPER_MAGIC_DRIVER_HEADER = 0x7FFF0008,
      COPPER_MAGIC_FPGA_HEADER = 0xFFFFFAFA,
      COPPER_MAGIC_FPGA_TRAILER = 0xFFFFF5F5,
      COPPER_MAGIC_DRIVER_TRAILER = 0x7FFF0009
    };

    ReducedRawHeader tmp_header;  //! Not record

    ReducedRawTrailer tmp_trailer; //! Not record


  protected :
    //! copy data
    void copyData(int* buf_to, int* pos_nwords_to, const int* buf_from,
                  const int copy_nwords, const int nwords_buf_to);



    ClassDef(PreRawCOPPER, 2);
    //ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
  };



  inline int* PreRawCOPPER::GetRawHdrBufPtr(int n)
  {
    int pos_nwords = GetBufferPos(n);
    return &(m_buffer[ pos_nwords ]);
  }

  inline int* PreRawCOPPER::GetRawTrlBufPtr(int n)
  {
    int pos_nwords;
    ReducedRawTrailer trl;

    if (n == (m_num_events * m_num_nodes) - 1) {
      pos_nwords = m_nwords - trl.GetTrlNwords();
    } else {
      pos_nwords = GetBufferPos(n + 1) - trl.GetTrlNwords();
    }
    return &(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PreRawCOPPER::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - ReducedRawTrailer::RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }

  inline int PreRawCOPPER::GetCOPPERNodeId(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.POS_SUBSYS_ID;
    return m_buffer[ pos_nwords ];
  }


  inline int PreRawCOPPER::GetExpNo(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetExpNo();
  }

  inline int PreRawCOPPER::GetRunNo(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetRunNo();
  }


  inline int PreRawCOPPER::GetSubRunNo(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetSubRunNo();
  }

  inline int PreRawCOPPER::GetRunNoSubRunNo(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetRunNoSubRunNo();
  }

  inline unsigned int PreRawCOPPER::GetEveNo(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetEveNo();
  }


  inline int PreRawCOPPER::GetSubsysId(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetSubsysId();
  }


  inline int PreRawCOPPER::GetDataType(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetDataType();
  }

  inline int PreRawCOPPER::GetTruncMask(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetTruncMask();
  }


  inline unsigned int PreRawCOPPER::GetCOPPERCounter(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + POS_EVE_NUM_COPPER + hdr.RAWHEADER_NWORDS;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }




  inline int PreRawCOPPER::Get1stDetectorNwords(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int nwords = 0;
    if (Get1stFINESSENwords(n) > 0) {
      nwords = Get1stFINESSENwords(n) -  SIZE_B2LHSLB_HEADER - SIZE_B2LFEE_HEADER
               - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
    }
    return nwords;
  }

  inline int PreRawCOPPER::Get2ndDetectorNwords(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int nwords = 0;
    if (Get2ndFINESSENwords(n) > 0) {
      nwords = Get2ndFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
               SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
    }
    return nwords;
  }

  inline int PreRawCOPPER::Get3rdDetectorNwords(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int nwords = 0;
    if (Get3rdFINESSENwords(n) > 0) {
      nwords = Get3rdFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
               SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
    }
    return nwords;
  }

  inline int PreRawCOPPER::Get4thDetectorNwords(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int nwords = 0;
    if (Get4thFINESSENwords(n) > 0) {
      nwords = Get4thFINESSENwords(n) -  SIZE_B2LHSLB_HEADER -
               SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
    }
    return nwords;
  }



  inline int PreRawCOPPER::Get1stFINESSENwords(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int PreRawCOPPER::Get2ndFINESSENwords(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int PreRawCOPPER::Get3rdFINESSENwords(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int PreRawCOPPER::Get4thFINESSENwords(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_D_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }






  inline int PreRawCOPPER::GetOffset1stFINESSE(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
    return pos_nwords;
  }

  inline int PreRawCOPPER::GetOffset2ndFINESSE(int n)
  {
    return GetOffset1stFINESSE(n) + Get1stFINESSENwords(n);
  }

  inline int PreRawCOPPER::GetOffset3rdFINESSE(int n)
  {
    return GetOffset2ndFINESSE(n) + Get2ndFINESSENwords(n);
  }

  inline int PreRawCOPPER::GetOffset4thFINESSE(int n)
  {
    return GetOffset3rdFINESSE(n) + Get3rdFINESSENwords(n);
  }



  inline int* PreRawCOPPER::Get1stFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset1stFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* PreRawCOPPER::Get2ndFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset2ndFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* PreRawCOPPER::Get3rdFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset3rdFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }

  inline int* PreRawCOPPER::Get4thFINESSEBuffer(int n)
  {
    int pos_nwords = GetOffset4thFINESSE(n);
    if (pos_nwords >= m_nwords) {
      printf("[DEBUG] Data size is smaller than data position info. Exting...\n");
      exit(1);
    }
    return &(m_buffer[ pos_nwords]);
  }






  inline int* PreRawCOPPER::Get1stDetectorBuffer(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    if (Get1stFINESSENwords(n) > 0) {
      int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
      return &(m_buffer[ pos_nwords ]);
    }
    return NULL;
  }

  inline int* PreRawCOPPER::Get2ndDetectorBuffer(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    if (Get2ndFINESSENwords(n) > 0) {
      int pos_nwords = GetOffset2ndFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
      return &(m_buffer[ pos_nwords ]);
    }
    return NULL;
  }

  inline int* PreRawCOPPER::Get3rdDetectorBuffer(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    if (Get3rdFINESSENwords(n) > 0) {
      int pos_nwords = GetOffset3rdFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
      return &(m_buffer[ pos_nwords ]);
    }
    return NULL;
  }

  inline int* PreRawCOPPER::Get4thDetectorBuffer(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    if (Get4thFINESSENwords(n) > 0) {
      int pos_nwords = GetOffset4thFINESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
      return &(m_buffer[ pos_nwords ]);
    }
    return NULL;
  }

  inline int* PreRawCOPPER::GetExpRunBuf(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
    return &(m_buffer[ pos_nwords ]);
  }



  inline unsigned int PreRawCOPPER::GetMagicDriverHeader(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PreRawCOPPER::GetMagicFPGAHeader(int n)
  {
    ReducedRawHeader hdr;
    int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_2;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PreRawCOPPER::GetMagicFPGATrailer(int n)
  {
    ReducedRawTrailer trl;
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - trl.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), trl.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PreRawCOPPER::GetMagicDriverTrailer(int n)
  {
    ReducedRawTrailer trl;
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - trl.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int PreRawCOPPER::GetTTCtimeTRGType(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetTTCtimeTRGType();
  }

  inline unsigned int PreRawCOPPER::GetTTUtime(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetTTUtime();
  }


  inline int PreRawCOPPER::GetTTCtime(int n)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    return hdr.GetTTCtime();
  }


  inline void PreRawCOPPER::GetTTTimeVal(int n, struct timeval* tv)
  {
    ReducedRawHeader hdr;
    hdr.SetBuffer(GetBuffer(n));
    hdr.GetTTTimeVal(tv);
    return ;
  }



}

#endif
