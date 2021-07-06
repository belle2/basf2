/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PRERAWCOPPERFORMAT_LATEST_H
#define PRERAWCOPPERFORMAT_LATEST_H

// Includes
#include <rawdata/dataobjects/PostRawCOPPERFormat_latest.h>
#include <rawdata/CRCCalculator.h>

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2


namespace Belle2 {

  /**
   * The Raw COPPER class ver.1 ( the latest version since May, 2014 )
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class PreRawCOPPERFormat_latest : public RawCOPPERFormat_latest {
  public:
    //! Default constructor
    PreRawCOPPERFormat_latest();

    //! Constructor using existing pointer to raw data buffer
    //PreRawCOPPERFormat_latest(int* bufin, int nwords);
    //! Destructor
    virtual ~PreRawCOPPERFormat_latest();

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

    //! get a checksum on trailer
    /* cppcheck-suppress missingOverride */
    unsigned int GetTrailerChksum(int  n) OVERRIDE_CPP17;

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

    //! check magic words of B2link HSLB header/trailer
    int CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords);

    //! reduce and merge header/trailer
    int CalcReducedDataSize(int* bufin, int nwords, int num_events, int num_nodes);
    //    int CalcReducedDataSize(RawDataBlock* raw_datablk);

    //! reduce and merge header/trailer
    void CopyReducedData(int* bufin, int nwords, int num_events, int num_nodes, int* buf_to, int* nwords_to);
    //    void CopyReducedData(RawDataBlock* raw_datablk, int* buf_to, int delete_flag_from);

    //! calculate reduced data size
    int CalcReducedNwords(int n);

    //! copy data to reduced buffer
    int CopyReducedBuffer(int n, int* buf_to);

    //! check CRC16 in B2LFEE trailer
    int CheckCRC16(int n, int finesse_num);

    //! Pack data (format ver. = -1 -> Select the latest format version)
    /* cppcheck-suppress missingOverride */
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* detector_buf_1st, int nwords_1st,
                         int* detector_buf_2nd, int nwords_2nd,
                         int* detector_buf_3rd, int nwords_3rd,
                         int* detector_buf_4th, int nwords_4th,
                         RawCOPPERPackerInfo rawcprpacker_info) OVERRIDE_CPP17;

    //! Pack data (format ver. = -1 -> Select the latest format version)
    /* cppcheck-suppress missingOverride */
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* const(&detector_buf_ch)[MAX_PCIE40_CH],
                         int const(&nwords_ch)[MAX_PCIE40_CH],
                         RawCOPPERPackerInfo rawcpr_info) OVERRIDE_CPP17;

    //! Get a pointer to detector buffer
    /* cppcheck-suppress missingOverride */
    int* GetDetectorBuffer(int n, int finesse_num) OVERRIDE_CPP17;

    //
    // Data Format : "B2Link HSLB Header"
    //
    enum {
      POS_MAGIC_B2LHSLB = 0,
      //      POS_EVE_CNT_B2LHSLB = 1,
      SIZE_B2LHSLB_HEADER = 1
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
      POS_TT_CTIME_B2LFEE = 0,
      POS_CHKSUM_B2LFEE = 1,
      SIZE_B2LFEE_TRAILER = 2
    };


    //
    // Data Format : "B2Link HSLB Trailer"
    //
    enum {
      POS_CHKSUM_B2LHSLB = 0,
      SIZE_B2LHSLB_TRAILER = 1
    };

    //
    // magic words attached by HSLB
    //
    enum {
      B2LHSLB_HEADER_MAGIC = 0xFFAA0000,
      B2LHSLB_TRAILER_MAGIC = 0xFF550000
    };

    //! data fromat after size reduction
    PostRawCOPPERFormat_latest m_reduced_rawcpr; //! not record

  protected :
    ///ver.2 Change FEE format as presented at B2GM in Nov.2013 ( Nov.20, 2013)
    //    ClassDefOverride(PreRawCOPPERFormat_latest, 2);

  };


  inline int* PreRawCOPPERFormat_latest::GetExpRunSubrunBuf(int n)
  {
#ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2
    CheckB2LFEEHeaderVersion(n);
#endif
    int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
    return &(m_buffer[ pos_nwords ]);
  }



  inline unsigned int PreRawCOPPERFormat_latest::GetMagicDriverHeader(int/* n */)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);
    B2FATAL(err_buf);
    return 0;
  }

  inline unsigned int PreRawCOPPERFormat_latest::GetMagicFPGAHeader(int/* n */)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);
    B2FATAL(err_buf);
    return 0;
  }

  inline unsigned int PreRawCOPPERFormat_latest::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), trl.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PreRawCOPPERFormat_latest::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int PreRawCOPPERFormat_latest::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }


  inline unsigned int PreRawCOPPERFormat_latest::GetCOPPERCounter(int/* n */)
  {
    char err_buf[500];
    sprintf(err_buf, "[FATAL] This function is not supported. Exiting...: \n%s %s %d\n",
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("[DEBUG] %s\n", err_buf);
    B2FATAL(err_buf);
    return 0;
  }

  inline unsigned int PreRawCOPPERFormat_latest::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline int* PreRawCOPPERFormat_latest::GetDetectorBuffer(int n, int finesse_num)
  {
    if (GetFINESSENwords(n, finesse_num) > 0) {
      return (GetFINESSEBuffer(n, finesse_num) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER);
    }
    return NULL;
  }

  inline int PreRawCOPPERFormat_latest::GetDetectorNwords(int n, int finesse_num)
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
