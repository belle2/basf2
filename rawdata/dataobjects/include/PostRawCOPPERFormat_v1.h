//+
// File : PostRawCOPPERFormat_v1.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef POSTRAWCOPPERFORMAT_V1_H
#define POSTRAWCOPPERFORMAT_V1_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>

#include <rawdata/dataobjects/RawCOPPERFormat_v1.h>
#include <rawdata/CRCCalculator.h>


#include <TObject.h>

// version #
#define POST_RAWCOPPER_FORMAT_VER1 1

//#define USE_B2LFEE_FORMAT_BOTH_VER1_AND_2

namespace Belle2 {

  /**
   * The Raw COPPER class ver.1 ( the latest version since May, 2014 )
   * This class stores data received by COPPER via belle2linkt
   * Data from all detectors except PXD are stored in this class
   */
  class PostRawCOPPERFormat_v1 : public RawCOPPERFormat_v1 {
  public:
    //! Default constructor
    PostRawCOPPERFormat_v1();

    //! Constructor using existing pointer to raw data buffer
    //PostRawCOPPERFormat_v1(int* bufin, int nwords);
    //! Destructor
    virtual ~PostRawCOPPERFormat_v1();

    //
    // Get position of or pointer to data
    //
    ///////////////////////////////////////////////////////////////////////////////////////
    // POINTER TO "DETECTOR BUFFER"
    //( after removing "B2link headers" from "FINESSE buffer". THIS IS THE RAW DATA FROM A DETECTOR
    ///////////////////////////////////////////////////////////////////////////////////////

    //! get Detector buffer length
    int GetDetectorNwords(int n, int finesse_num);

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
    //    virtual int GetBufferPos(int n);

    //
    // Get information from 13words "COPPER header" attached by COPPER board
    //
    //! get COPPER counter(not event number)
    unsigned int GetCOPPERCounter(int n);

    //! get # of offset words for FINESSE slot A buffer position
    int GetOffset1stFINESSE(int n);

    //! get data size of  FINESSE buffer
    int GetFINESSENwords(int n, int finesse);

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

    //! should be called by DeSerializerCOPPER.cc and fill contents in RawHeader
    unsigned int FillTopBlockRawHeader(unsigned int m_node_id, unsigned int prev_eve32,
                                       unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no);

    //! read COPPER driver's checksum value
    unsigned int GetDriverChkSum(int n);

    //! calc COPPER driver's checksum value
    unsigned int CalcDriverChkSum(int n);

    //! check data contents
    void CheckData(int n,
                   unsigned int prev_evenum, unsigned int* cur_evenum,
                   unsigned int prev_copper_ctr, unsigned int* cur_copper_ctr,
                   unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no);

    //! check data contents
    void CheckUtimeCtimeTRGType(int n);

    //! check magic words
    int CheckB2LHSLBMagicWords(int* finesse_buf, int finesse_nwords);

    //! check magic words
    int CheckCRC16(int n, int finesse_num);

    //! Pack data (format ver. = -1 -> Select the latest format version)
    int* PackDetectorBuf(int* packed_buf_nwords,
                         int* detector_buf_1st, int nwords_1st,
                         int* detector_buf_2nd, int nwords_2nd,
                         int* detector_buf_3rd, int nwords_3rd,
                         int* detector_buf_4th, int nwords_4th,
                         RawCOPPERPackerInfo rawcprpacker_info);

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
      POS_B2LFEE_CRC16 = 0,
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
    //    ClassDef(PostRawCOPPERFormat_v1, 2);

  };

  inline int PostRawCOPPERFormat_v1::GetOffset1stFINESSE(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
    return pos_nwords;
  }

  inline int* PostRawCOPPERFormat_v1::Get1stDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v1::Get2ndDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v1::Get3rdDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v1::Get4thDetectorBuffer(int n)
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

  inline int* PostRawCOPPERFormat_v1::GetExpRunSubrunBuf(int n)
  {
    int pos_nwords = GetBufferPos(n) + tmp_header.POS_EXP_RUN_NO;
    return &(m_buffer[ pos_nwords ]);
    /* #ifdef USE_B2LFEE_FORMAT_BOTH_VER1_AND_2 */
    /*     CheckB2LFEEHeaderVersion(n); */
    /* #endif */
    /*     int pos_nwords = GetOffset1stFINESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN; */
    /*     return &(m_buffer[ pos_nwords ]); */
  }

  inline unsigned int PostRawCOPPERFormat_v1::GetDriverChkSum(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n)
                     - tmp_trailer.RAWTRAILER_NWORDS - SIZE_COPPER_DRIVER_TRAILER;
    return m_buffer[ pos_nwords ];
  }


  inline unsigned int PostRawCOPPERFormat_v1::GetCOPPERCounter(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "This data format does not have COPPER counter.(block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_v1::GetMagicDriverHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }

  inline unsigned int PostRawCOPPERFormat_v1::GetMagicFPGAHeader(int n)
  {
    char err_buf[500];
    sprintf(err_buf, "This function is not supported. (block %d) Exiting...\n %s %s %d\n",
            n, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    std::string err_str = err_buf; throw (err_str);
    return 0;
  }


  inline unsigned int PostRawCOPPERFormat_v1::GetMagicFPGATrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 3;

    //    printf( "[DEBUG] 1 %d 2 %d 3 %d\n", GetBufferPos(n), GetBlockNwords(n), tmp_trailer.GetTrlNwords());

    return (unsigned int)(m_buffer[ pos_nwords ]);
  }

  inline unsigned int PostRawCOPPERFormat_v1::GetMagicDriverTrailer(int n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() - 1;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }


  inline unsigned int PostRawCOPPERFormat_v1::GetTrailerChksum(int  n)
  {
    int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - tmp_trailer.GetTrlNwords() + tmp_trailer.POS_CHKSUM;
    return (unsigned int)(m_buffer[ pos_nwords ]);
  }




}

#endif
