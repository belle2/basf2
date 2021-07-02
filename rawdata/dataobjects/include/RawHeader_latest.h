/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawHeader_latest.h
// Description : Module to handle RawHeader_latest attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWHEADER_LATEST_H
#define RAWHEADER_LATEST_H

#include <stdio.h>
#include <string.h>
#include <rawdata/switch_basf2_standalone.h>


//#define HEADER_SIZE 16

namespace Belle2 {

  /**
   * The Raw Header class ver.1 ( the latest version since May, 2014 )
   * This class defines the format of the header of RawCOPPER class data
   * and used for extracting header info from RawCOPPER object
   */

  //  class RawHeader_latest : public TObject {
  class RawHeader_latest {
  public:
    //! Default constructor
    RawHeader_latest();

    //! Constructor using existing pointer to raw data buffer
    explicit RawHeader_latest(int*);

    //! Destructor
    ~RawHeader_latest();

    //! Get header contents
    int* GetBuffer() { return m_buffer; }

    //! set buffer
    void SetBuffer(int* bufin) { m_buffer = bufin; }

    //! initialize header
    void CheckSetBuffer();

    //! check if m_buffer exists
    void CheckGetBuffer();

    //! set contents of header
    void SetNwords(int nwords);

    //! set contents of header
    void SetEveNo(unsigned int eve_no);

    //! set contents of header
    void SetNodeID(unsigned int node_id);

    //! set contents of header
    void SetDataType(int data_type);

    //! set contents of header
    void SetTruncMask(int trunc_mask);

    //    void SetB2LFEEHdrPart(unsigned int word1, unsigned int word2);   //! set contents of header
    //     void SetFTSW2Words(int* ftsw_buf);

    /// Set values of FTSW info( trigger timing)
    void SetFTSW2Words(unsigned int word1, unsigned int word2);

    /// Set a word consists of exp #, run # and subrun #
    void SetExpRunNumber(int* exprun_buf);

    //! set contents of header
    void SetOffset1stFINESSE(int offset_1st_FINESSE);    //! set contents of header

    void SetOffset2ndFINESSE(int offset_2nd_FINESSE);    //! set contents of header

    void SetOffset3rdFINESSE(int offset_3rd_FINESSE);    //! set contents of header

    void SetOffset4thFINESSE(int offset_4th_FINESSE);    //! set contents of header

    //    void SetMagicWordEntireHeader(); //! set magic words;

    /// Add nodeinfo in trace area
    //    int AddNodeInfo(int node_id);

    int GetNwords();  //! get contents of header

    int GetHdrNwords();  //! get contents of header

    unsigned int GetExpRunSubrun(); //! get a run/exp number combined word

    int GetExpNo();  //! get contents of header

    int GetRunNo();    //! get run # (14bit)

    int GetSubRunNo();    //! get restart #(8bit)

    unsigned int GetEveNo();  //! get contents of header

    unsigned int GetNodeID();  //! get contents of header

    int GetDataType();  //! get contents of header

    int GetTruncMask();  //! get contents of header

    //! Get Detected Error bitflag
    unsigned int GetErrorBitFlag();

    //! Add Detected Error bitflag
    void AddErrorBitFlag(unsigned int error_bit_flag);

    //! Get Packet CRC error flag
    int GetPacketCRCError();

    //! Get Detected Error bitflag
    int GetEventCRCError();

    //! get contents of header
    int GetOffset1stFINESSE();

    //! get contents of header
    int GetOffset2ndFINESSE();

    //! get contents of header
    int GetOffset3rdFINESSE();

    //! get contents of header
    int GetOffset4thFINESSE();

    //    int GetNumNodes();  //! get contents of header

    //    int GetNodeInfo(int node_no, int* node_id);    //! get contents of header

    //! get contents of header
    unsigned int GetTTCtimeTRGType();

    //! get contents of header
    int GetTTCtime();

    //! get TRGType
    int GetTRGType();

    //! get contents of header
    unsigned int GetTTUtime();

    //! get contents of header
    void GetTTTimeVal(struct  timeval* tv);

    /// Get magic word to check the data corruption
    //    unsigned int GetMagicWordEntireHeader();

    //! check the contents of header
    void CheckHeader(int* buf);

    /*
      Experimental(10bit) #, Run#(14bit), restat# bit mask(8bit)
    */
    enum {
      MAGIC_WORD = 0x7F7F0000,
      MAGIC_MASK = 0xFFFF0000,
      MAGIC_SHIFT = 16,
      FORMAT_VERSION__MASK = 0x0000FF00,
      FORMAT_VERSION_SHIFT = 8
    };

    enum {
      EXP_MASK = 0xFFC00000,
      EXP_SHIFT = 22,
      RUNNO_MASK = 0x003FFF00,
      RUNNO_SHIFT = 8,
      SUBRUNNO_MASK = 0x000000FF
    };

    enum {
      RAWHEADER_NWORDS = 56
    };

    enum {
      HDR_NWORDS_MASK = 0x000000FF
    };

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_VERSION_HDRNWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_TTCTIME_TRGTYPE = 4,
      POS_TTUTIME = 5,
      POS_NODE_ID = 6,
      POS_TRUNC_MASK_DATATYPE = 7,
      POS_CH_POS_TABLE = 8
    };


    /* Data Format : Node info */
    /*     enum { */
    /*       NUM_MAX_NODES = 4   /\* (NUM_MAX_NODES + 1) slots are available in m_buffer. */
    /*          (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes */
    /*          exceeds NUM_MAX_NODES *\/ */
    /*     }; */

    /*     /\* Data Format : Magic word *\/ */
    /*     enum { */
    /*       MAGIC_WORD_TERM_HEADER = 0x7fff0005 */
    /*     }; */



    /* To extract ctime */
    enum {
      TTCTIME_MASK = 0x7FFFFFF0,
      TTCTIME_SHIFT = 4,
      TRGTYPE_MASK = 0xF
    };

    /* Error bit in POS_TRUNC_MASK_DATATYPE */
    /* Changed from the position of error bit to actual error value ( 0 -> 0x1, 1 -> 0x2 ) */
    enum {
      B2LINK_PACKET_CRC_ERROR = 0x1,
      B2LINK_EVENT_CRC_ERROR = 0x2
    };

  private:
    //! do not record buffer ( RawCOPPER includes buffer of RawHeader_latest and RawTrailer )
    int* m_buffer; //! do not record

    /// To derive from TObject
    // ver.2 Do not record m_buffer pointer. (Dec.19, 2014)
    //    ClassDef(RawHeader_latest, 1);
  };


  inline void RawHeader_latest::CheckSetBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Exiting...");
    }
  }

  inline void RawHeader_latest::CheckGetBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Data is corrupted or header info has not yet filled. Exiting...");
//  } else if (m_buffer[ POS_TERM_HEADER ] != MAGIC_WORD_TERM_HEADER) {
//    B2FATAL("magic word is invalid(0x"<< std::hex() << m_buffer[ POS_TERM_HEADER ] <<"). Data is corrupted or header info has not yet filled. Exiting...");
    }
  }


  inline void RawHeader_latest::SetNwords(int nwords)
  {
    CheckSetBuffer();
    m_buffer[ POS_NWORDS ] = nwords;

  }

  inline void RawHeader_latest::SetEveNo(unsigned int eve_no)
  {
    CheckSetBuffer();
    m_buffer[ POS_EVE_NO ] = eve_no;
  }

  inline void RawHeader_latest::SetNodeID(unsigned int node_id)
  {
    CheckSetBuffer();
    m_buffer[ POS_NODE_ID ] = (int)node_id;
  }

  inline void RawHeader_latest::SetDataType(int data_type)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
      (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
  }

  inline void RawHeader_latest::SetTruncMask(int trunc_mask)
  {
    CheckSetBuffer();
    /* cppcheck-suppress shiftTooManyBitsSigned */
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline void RawHeader_latest::SetFTSW2Words(unsigned int word1,
                                              unsigned int word2)
  {
    CheckSetBuffer();
    m_buffer[ POS_TTCTIME_TRGTYPE ] = word1;
    m_buffer[ POS_TTUTIME ] = word2;
    return;
  }


  inline void RawHeader_latest::SetExpRunNumber(int* exprun_buf)
  {
    CheckSetBuffer();
    memcpy(&(m_buffer[ POS_EXP_RUN_NO ]), (char*)exprun_buf, sizeof(int) * 1);
    return;
  }


//
// Obtain info
//

  inline int RawHeader_latest::GetNwords()
  {
    CheckGetBuffer();
    return m_buffer[ POS_NWORDS ];
  }

  inline int RawHeader_latest::GetHdrNwords()
  {

    //    CheckGetBuffer();
    //    return m_buffer[ POS_HDR_NWORDS ];
    return RAWHEADER_NWORDS;
  }

  inline int RawHeader_latest::GetExpNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }


  inline int RawHeader_latest::GetRunNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawHeader_latest::GetSubRunNo()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline unsigned int RawHeader_latest::GetExpRunSubrun()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]));
  }


  inline unsigned int RawHeader_latest::GetEveNo()
  {
    CheckGetBuffer();
    return m_buffer[ POS_EVE_NO ];
  }

  inline unsigned int RawHeader_latest::GetNodeID()
  {
    CheckGetBuffer();
    return (unsigned int)m_buffer[ POS_NODE_ID ];
  }

  inline int RawHeader_latest::GetDataType()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline int RawHeader_latest::GetTruncMask()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
  }

  inline unsigned int RawHeader_latest::GetErrorBitFlag()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TRUNC_MASK_DATATYPE ]);
  }

  inline void RawHeader_latest::AddErrorBitFlag(unsigned int error_bit_flag)
  {
    CheckGetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] |= (int)error_bit_flag;
    return;
  }

  inline int RawHeader_latest::GetPacketCRCError()
  {
    CheckGetBuffer();
    unsigned int temp_err_flag = GetErrorBitFlag();
    if ((temp_err_flag & B2LINK_PACKET_CRC_ERROR) == 0) {
      return 0;
    }
    return 1;
  }

  inline int RawHeader_latest::GetEventCRCError()
  {
    CheckGetBuffer();
    unsigned int temp_err_flag = GetErrorBitFlag();
    if ((temp_err_flag & B2LINK_EVENT_CRC_ERROR) == 0) {
      return 0;
    }
    return 1;
  }


  /*   inline int RawHeader_latest::GetNumNodes() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_NUM_NODES ]; */
  /*   } */

  inline unsigned int RawHeader_latest::GetTTCtimeTRGType()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTCTIME_TRGTYPE ]);
  }

  inline int RawHeader_latest::GetTTCtime()
  {
    CheckGetBuffer();
    return (int)((GetTTCtimeTRGType() & TTCTIME_MASK) >> TTCTIME_SHIFT);
  }

  inline int RawHeader_latest::GetTRGType()
  {
    CheckGetBuffer();
    return (int)(GetTTCtimeTRGType() & TRGTYPE_MASK);
  }

  inline unsigned int RawHeader_latest::GetTTUtime()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTUTIME ]);
  }

  inline void RawHeader_latest::GetTTTimeVal(struct timeval* tv)
  {
    tv->tv_sec = GetTTUtime();
    tv->tv_usec = (int)(((double)GetTTCtime()) / 127.216);
    return ;
  }


  /*   inline unsigned int RawHeader_latest::GetMagicWordEntireHeader() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_TERM_HEADER ]; */
  /*   } */





}

#endif
