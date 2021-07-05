/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWCOPPERFORMAT_V0_H
#define RAWCOPPERFORMAT_V0_H

// Includes
#include <stdio.h>

#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <rawdata/dataobjects/RawHeader_v0.h>
#include <rawdata/dataobjects/RawTrailer_v0.h>

/* #include <framework/datastore/DataStore.h> */
/* #include <TObject.h> */

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

namespace Belle2 {

  /**
   * The Raw COPPER class ver.0 ( from August, 2013 to April, 2014 )
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class RawCOPPERFormat_v0 : public RawCOPPERFormat {
  public:
    //! Default constructor
    RawCOPPERFormat_v0();

    //! Constructor using existing pointer to raw data buffer
    //RawCOPPERFormat_v0(int* bufin, int nwords);
    //! Destructor
    virtual ~RawCOPPERFormat_v0();

    //
    // Get position of or pointer to data
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////


    //! get Detector buffer length
    /* cppcheck-suppress missingOverride */
    int GetDetectorNwords(int n, int finesse_num) OVERRIDE_CPP17;

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

    //! get Detector buffer of slot A
    /* cppcheck-suppress missingOverride */
    int* Get1stDetectorBuffer(int n) OVERRIDE_CPP17;

    //! get Detector Buffer of slot B
    /* cppcheck-suppress missingOverride */
    int* Get2ndDetectorBuffer(int n) OVERRIDE_CPP17;

    //! get Detector Buffer of slot C
    /* cppcheck-suppress missingOverride */
    int* Get3rdDetectorBuffer(int n) OVERRIDE_CPP17;

    //! get Detector Buffer of slot D
    /* cppcheck-suppress missingOverride */
    int* Get4thDetectorBuffer(int n) OVERRIDE_CPP17;
    ///////////////////////////////////////////////////////////////////////////////////////

    /*     //! get posistion of COPPER block in unit of word */
    /*     virtual int GetBufferPos(int n); */

    //! get buffer pointer of rawcopper trailer
    /* cppcheck-suppress missingOverride */
    virtual int* GetRawTrlBufPtr(int n) OVERRIDE_CPP17;

    //
    // Get information from "RawCOPPERFormat_v0 header" attached by DAQ software
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
    //! get COPPER counter(not event number)
    /* cppcheck-suppress missingOverride */
    virtual unsigned int GetCOPPERCounter(int n) OVERRIDE_CPP17;

    //! get # of FINNESEs which contains data
    /* cppcheck-suppress missingOverride */
    virtual int GetNumFINESSEBlock(int n) OVERRIDE_CPP17;

    //! get # of offset words for FINESSE slot A buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset1stFINESSE(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE buffer
    /* cppcheck-suppress missingOverride */
    int GetFINESSENwords(int n, int finesse) OVERRIDE_CPP17;

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

    //! get b2l block from "FEE b2link header"
    /* cppcheck-suppress missingOverride */
    virtual int* GetExpRunSubrunBuf(int n) OVERRIDE_CPP17;

    //! get b2l block from "FEE b2link header"
    /* cppcheck-suppress missingOverride */
    virtual unsigned int GetB2LFEE32bitEventNumber(int n) OVERRIDE_CPP17;

    //
    // read magic word to check data
    //
    //! get magic word of  COPPER driver header
    /* cppcheck-suppress missingOverride */
    unsigned int GetMagicDriverHeader(int n) OVERRIDE_CPP17;

    //! get magic word of  COPPER FPGA header
    /* cppcheck-suppress missingOverride */
    unsigned int GetMagicFPGAHeader(int n) OVERRIDE_CPP17;

    //! get magic word of  COPPER FPGA trailer
    /* cppcheck-suppress missingOverride */
    unsigned int GetMagicFPGATrailer(int n) OVERRIDE_CPP17;

    //! get magic word of  COPPER driver trailer
    /* cppcheck-suppress missingOverride */
    unsigned int GetMagicDriverTrailer(int n) OVERRIDE_CPP17;

    //! Get checksum in RawTrailer
    /* cppcheck-suppress missingOverride */
    unsigned int GetTrailerChksum(int  n) OVERRIDE_CPP17;

    //! Check if COPPER Magic words are correct
    /* cppcheck-suppress missingOverride */
    bool CheckCOPPERMagic(int n) OVERRIDE_CPP17;

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

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    /* cppcheck-suppress missingOverride */
    unsigned int FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
                                       unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) OVERRIDE_CPP17;

    //! read COPPER driver's checksum value
    /* cppcheck-suppress missingOverride */
    unsigned int GetDriverChkSum(int n) OVERRIDE_CPP17;

    //! calc COPPER driver's checksum value
    /* cppcheck-suppress missingOverride */
    unsigned int CalcDriverChkSum(int n) OVERRIDE_CPP17;

    //! check data contents
    /* cppcheck-suppress missingOverride */
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) OVERRIDE_CPP17;

    //! check data contents
    /* cppcheck-suppress missingOverride */
    void CheckUtimeCtimeTRGType(int n) OVERRIDE_CPP17;

    //! Get ctime
    /* cppcheck-suppress missingOverride */
    int GetTTCtime(int n) OVERRIDE_CPP17;

    //! Get trgtype
    /* cppcheck-suppress missingOverride */
    int GetTRGType(int n) OVERRIDE_CPP17;

    //! Get timeval
    /* cppcheck-suppress missingOverride */
    void GetTTTimeVal(int n, struct timeval* tv) OVERRIDE_CPP17;

    //! Pack data (format ver. = -1 -> Select the latest format version)
    /* cppcheck-suppress missingOverride */
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* detector_buf_1st, int nwords_1st,
                         int* detector_buf_2nd, int nwords_2nd,
                         int* detector_buf_3rd, int nwords_3rd,
                         int* detector_buf_4th, int nwords_4th,
                         RawCOPPERPackerInfo rawcprpacker_info) OVERRIDE_CPP17;

    //! Get the max number of channels in a readout board
    /* cppcheck-suppress missingOverride */
    int GetMaxNumOfCh(int n) OVERRIDE_CPP17;

    enum {
      DATA_FORMAT_VERSION = 0
    };

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

    //! header ( not recorded )
    RawHeader_v0 tmp_header;

    //! trailer ( not recorded )
    RawTrailer_v0 tmp_trailer;

  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(RawCOPPERFormat_v0, 2);

  };




  inline int* RawCOPPERFormat_v0::GetRawTrlBufPtr(int n)
  {
    int pos_nwords;
    if (n == (m_num_events * m_num_nodes) - 1) {
      pos_nwords = m_nwords - tmp_trailer.GetTrlNwords();
    } else {
      pos_nwords = GetBufferPos(n + 1) - tmp_trailer.GetTrlNwords();
    }
    return &(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_v0::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }

  inline int RawCOPPERFormat_v0::GetExpNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpNo();
  }

  inline unsigned int RawCOPPERFormat_v0::GetExpRunSubrun(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetExpRunSubrun();
  }

  inline int RawCOPPERFormat_v0::GetRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetRunNo();
  }


  inline int RawCOPPERFormat_v0::GetSubRunNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetSubRunNo();
  }

  inline unsigned int RawCOPPERFormat_v0::GetEveNo(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetEveNo();
  }


  inline unsigned int RawCOPPERFormat_v0::GetNodeID(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetNodeID();
  }


  inline int RawCOPPERFormat_v0::GetDataType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetDataType();
  }

  inline int RawCOPPERFormat_v0::GetTruncMask(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTruncMask();
  }

  inline unsigned int RawCOPPERFormat_v0::GetErrorBitFlag(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetErrorBitFlag();
  }

  inline void RawCOPPERFormat_v0::AddErrorBitFlag(int n, unsigned int error_bit_flag)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.AddErrorBitFlag(error_bit_flag);
    return;
  }

  inline unsigned int RawCOPPERFormat_v0::GetCOPPERCounter(int n)
  {
    int pos_nwords = GetBufferPos(n) + POS_EVE_NUM_COPPER + tmp_header.RAWHEADER_NWORDS;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }




  inline int RawCOPPERFormat_v0::Get1stDetectorNwords(int n)
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

  inline int RawCOPPERFormat_v0::Get2ndDetectorNwords(int n)
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

  inline int RawCOPPERFormat_v0::Get3rdDetectorNwords(int n)
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

  inline int RawCOPPERFormat_v0::Get4thDetectorNwords(int n)
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



  inline int RawCOPPERFormat_v0::Get1stFINESSENwords(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int RawCOPPERFormat_v0::Get2ndFINESSENwords(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int RawCOPPERFormat_v0::Get3rdFINESSENwords(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }

  inline int RawCOPPERFormat_v0::Get4thFINESSENwords(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_CH_D_DATA_LENGTH;
    return m_buffer[ pos_nwords ];
  }


  inline int RawCOPPERFormat_v0::GetOffset1stFINESSE(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
    return pos_nwords;
  }


  inline int* RawCOPPERFormat_v0::Get1stDetectorBuffer(int n)
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

  inline int* RawCOPPERFormat_v0::Get2ndDetectorBuffer(int n)
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

  inline int* RawCOPPERFormat_v0::Get3rdDetectorBuffer(int n)
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

  inline int* RawCOPPERFormat_v0::Get4thDetectorBuffer(int n)
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

  inline int* RawCOPPERFormat_v0::GetExpRunSubrunBuf(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
    return &(m_buffer[ pos_nwords ]);
  }



  inline unsigned int RawCOPPERFormat_v0::GetMagicDriverHeader(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_MAGIC_COPPER_1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_v0::GetMagicFPGAHeader(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + POS_MAGIC_COPPER_2;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_v0::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), tmp_trailer.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int RawCOPPERFormat_v0::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int RawCOPPERFormat_v0::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int RawCOPPERFormat_v0::GetTTCtimeTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtimeTRGType();
  }

  inline unsigned int RawCOPPERFormat_v0::GetTTUtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTUtime();
  }


  inline int RawCOPPERFormat_v0::GetTTCtime(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTTCtime();
  }

  inline int RawCOPPERFormat_v0::GetTRGType(int n)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    return tmp_header.GetTRGType();
  }

  inline void RawCOPPERFormat_v0::GetTTTimeVal(int n, struct timeval* tv)
  {
    tmp_header.SetBuffer(GetBuffer(n));
    tmp_header.GetTTTimeVal(tv);
    return ;
  }

  inline int RawCOPPERFormat_v0::GetMaxNumOfCh(int/* n */)
  {
    return MAX_COPPER_CH;
  }

}

#endif
