/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef POSTRAWCOPPERFORMAT_V2_H
#define POSTRAWCOPPERFORMAT_V2_H

// Includes
/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string> */
/* #include <sys/time.h> */

#include <rawdata/dataobjects/RawCOPPERFormat_v2.h>
#include <rawdata/CRCCalculator.h>

//#include <framework/datastore/DataStore.h>
//#include <TObject.h>

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

namespace Belle2 {

  /**
   * The Raw COPPER class ver.2
   * This class stores data received by COPPER via belle2link
   * Data from all detectors except PXD are stored in this class
   */
  class PostRawCOPPERFormat_v2 : public RawCOPPERFormat_v2 {
  public:
    //! Default constructor
    PostRawCOPPERFormat_v2();

    //! Constructor using existing pointer to raw data buffer
    //PostRawCOPPERFormat_v2(int* bufin, int nwords);
    //! Destructor
    virtual ~PostRawCOPPERFormat_v2();

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

    //! get posistion of COPPER block in unit of word
    //    virtual int GetBufferPos(int n);

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    /* cppcheck-suppress missingOverride */
    unsigned int GetCOPPERCounter(int n) OVERRIDE_CPP17;

    //! get # of offset words for FINESSE slot A buffer position
    /* cppcheck-suppress missingOverride */
    int GetOffset1stFINESSE(int n) OVERRIDE_CPP17;

    //! get data size of  FINESSE buffer
    /* cppcheck-suppress missingOverride */
    int GetFINESSENwords(int n, int finesse) OVERRIDE_CPP17;

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

    //! Get CRC16 value for an event
    /* cppcheck-suppress missingOverride */
    int GetEventCRC16Value(int n, int finesse_num) OVERRIDE_CPP17;

    //! Check if COPPER Magic words are correct
    /* cppcheck-suppress missingOverride */
    bool CheckCOPPERMagic(int n) OVERRIDE_CPP17;

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    /* cppcheck-suppress missingOverride */
    unsigned int FillTopBlockRawHeader(unsigned int m_node_id,
                                       unsigned int prev_eve32, unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no) OVERRIDE_CPP17;

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

    //! check magic words
    int CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords);

    //! check magic words
    int CheckCRC16(int n, int finesse_num);

    //! Pack data (format ver. = -1 -> Select the latest format version)
    /* cppcheck-suppress missingOverride */
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* detector_buf_1st, int nwords_1st,
                         int* detector_buf_2nd, int nwords_2nd,
                         int* detector_buf_3rd, int nwords_3rd,
                         int* detector_buf_4th, int nwords_4th,
                         RawCOPPERPackerInfo rawcprpacker_info) OVERRIDE_CPP17;

    //
    // size of "COPPER front header" and "COPPER trailer"
    //
    //! Copper data words = ( total_data_length in COPPER header ) + COPPER_HEADER_TRAILER_NWORDS

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

    // Data Format : "B2Link HSLB Header"
    enum {
      POS_B2LHSLB_MAGIC = 0,
      SIZE_B2LHSLB_HEADER = 1
    };


    // Data Format : "B2Link FEE Header"
    enum {
      POS_B2L_CTIME = 0,
      SIZE_B2LFEE_HEADER = 1
    };

    // Data Format : B2Link FEE Trailer
    enum {
      POS_B2LFEE_ERRCNT_CRC16 = 0,
      SIZE_B2LFEE_TRAILER = 1
    };

    //
    // Data Format : "B2Link HSLB Trailer"
    //
    enum {
      SIZE_B2LHSLB_TRAILER = 0
    };



  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(PostRawCOPPERFormat_v2, 2);

  };

  inline int PostRawCOPPERFormat_v2::GetOffset1stFINESSE(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
    return pos_nwords;
  }

  inline int* PostRawCOPPERFormat_v2::Get1stDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v2::Get2ndDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v2::Get3rdDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v2::Get4thDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v2::GetExpRunSubrunBuf(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.POS_EXP_RUN_NO;
    return &(m_buffer[ pos_nwords ]);
    /* #ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2 */
    /*     CheckB2LFEEHeaderVersion(n); */
    /* #endif */
    /*     int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN; */
    /*     return &(m_buffer[ pos_nwords ]); */
  }

  inline unsigned int PostRawCOPPERFormat_v2::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }


  inline unsigned int PostRawCOPPERFormat_v2::GetCOPPERCounter(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This data format does not have COPPER counter.(block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_v2::GetMagicDriverHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_v2::GetMagicFPGAHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }


  inline unsigned int PostRawCOPPERFormat_v2::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), tmp_trailer.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PostRawCOPPERFormat_v2::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int PostRawCOPPERFormat_v2::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline int PostRawCOPPERFormat_v2::GetEventCRC16Value(int n, int finesse_num)
  {
    int fin_nwords = GetFINESSENwords(n, finesse_num);
    if (fin_nwords > 0) {
      int* buf = GetFINESSEBuffer(n, finesse_num) +  fin_nwords
                 - ((SIZE_B2LFEE_TRAILER - POS_B2LFEE_ERRCNT_CRC16) + SIZE_B2LHSLB_TRAILER) ;
      return (int)(*buf & 0xffff);
    }
    return -1;
  }
}

#endif
