//+
// File : PostRawCOPPERFormat_latest.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef POSTRAWCOPPERFORMAT_LATEST_H
#define POSTRAWCOPPERFORMAT_LATEST_H

// Includes
/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string> */
/* #include <sys/time.h> */

#include <rawdata/dataobjects/RawCOPPERFormat_latest.h>
#include <rawdata/CRCCalculator.h>

//#include <framework/datastore/DataStore.h>
//#include <TObject.h>

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

namespace Belle2 {

  /**
   * The Raw COPPER class ver. 4 (the 1st ver. for PCIe40 data format)
   * This class stores data received by PCIe40 via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class PostRawCOPPERFormat_latest : public RawCOPPERFormat_latest {
  public:
    //! Default constructor
    PostRawCOPPERFormat_latest();

    //! Constructor using existing pointer to raw data buffer
    //PostRawCOPPERFormat_latest(int* bufin, int nwords);
    //! Destructor
    virtual ~PostRawCOPPERFormat_latest();

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

    ///////////////////////////////////////////////////////////////////////////////////////

    //! get posistion of COPPER block in unit of word
    //    virtual int GetBufferPos(int n);

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    /* cppcheck-suppress missingOverride */
    unsigned int GetCOPPERCounter(int n) OVERRIDE_CPP17;

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

    //! Pack data for PCIe40 data-format
    /* cppcheck-suppress missingOverride */
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* const(&detector_buf_ch)[MAX_PCIE40_CH],
                         int const(&nwords_ch)[MAX_PCIE40_CH],
                         RawCOPPERPackerInfo rawcpr_info) OVERRIDE_CPP17;

    //! Get a pointer to detector buffer
    /* cppcheck-suppress missingOverride */
    int* GetDetectorBuffer(int n, int finesse_num) OVERRIDE_CPP17;

    // Data Format : "B2Link PCIe40 ch Header"
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
      POS_B2LHSLB_TRL_MAGIC = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };



  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(PostRawCOPPERFormat_latest, 2);

  };

  inline int* PostRawCOPPERFormat_latest::GetExpRunSubrunBuf(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.POS_EXP_RUN_NO;
    return &(m_buffer[ pos_nwords ]);
    /* #ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2 */
    /*     CheckB2LFEEHeaderVersion(n); */
    /* #endif */
    /*     int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN; */
    /*     return &(m_buffer[ pos_nwords ]); */
  }

  inline unsigned int PostRawCOPPERFormat_latest::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }


  inline unsigned int PostRawCOPPERFormat_latest::GetCOPPERCounter(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This data format does not have COPPER counter.(block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_latest::GetMagicDriverHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_latest::GetMagicFPGAHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }


  inline unsigned int PostRawCOPPERFormat_latest::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), tmp_trailer.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PostRawCOPPERFormat_latest::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int PostRawCOPPERFormat_latest::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline int PostRawCOPPERFormat_latest::GetEventCRC16Value(int n, int finesse_num)
  {
    int fin_nwords = GetFINESSENwords(n, finesse_num);
    if (fin_nwords > 0) {
      int* buf = GetFINESSEBuffer(n, finesse_num) +  fin_nwords
                 - ((SIZE_B2LFEE_TRAILER - POS_B2LFEE_ERRCNT_CRC16) + SIZE_B2LHSLB_TRAILER) ;
      return (int)(*buf & 0xffff);
    }
    return -1;
  }

  inline int* PostRawCOPPERFormat_latest::GetDetectorBuffer(int n, int finesse_num)
  {
    if (GetFINESSENwords(n, finesse_num) > 0) {
      return (GetFINESSEBuffer(n, finesse_num) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER);
    }
    return NULL;
  }


  inline int PostRawCOPPERFormat_latest::GetDetectorNwords(int n, int finesse_num)
  {
    int nwords = 0;
    if (GetFINESSENwords(n, finesse_num) > 0) {
      nwords = GetFINESSENwords(n, finesse_num)
               - (SIZE_B2LHSLB_HEADER + SIZE_B2LHSLB_TRAILER +  SIZE_B2LFEE_HEADER + SIZE_B2LFEE_TRAILER);
    }
    return nwords;
  }


}

#endif
