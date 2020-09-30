//+
// File : RawHeader_v1.h
// Description : Module to handle RawHeader_v1 attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWHEADER_V1_H
#define RAWHEADER_V1_H

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

  class RawHeader_v1  {
    //  class RawHeader_v1 : public TObject {
  public:
    //! Default constructor
    RawHeader_v1();

    //! Constructor using existing pointer to raw data buffer
    explicit RawHeader_v1(int*);

    //! Destructor
    ~RawHeader_v1();

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
      RAWHEADER_NWORDS = 12
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
      POS_OFFSET_1ST_FINESSE = 8,
      POS_OFFSET_2ND_FINESSE = 9,
      POS_OFFSET_3RD_FINESSE = 10,
      POS_OFFSET_4TH_FINESSE = 11,
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

  private:
    //! do not record buffer ( RawCOPPER includes buffer of RawHeader_v1 and RawTrailer )
    int* m_buffer; //! do not record

    /// To derive from TObject
    // ver.2 Do not record m_buffer pointer. (Dec.19, 2014)
    //    ClassDef(RawHeader_v1, 1);
  };


  inline void RawHeader_v1::CheckSetBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Exiting...");
    }
  }

  inline void RawHeader_v1::CheckGetBuffer()
  {

    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Data is corrupted or header info has not yet filled. Exiting...");
//  } else if (m_buffer[ POS_TERM_HEADER ] != MAGIC_WORD_TERM_HEADER) { */
//    B2FATAL("[DEBUG]  magic word is invalid(0x" << hex() << m_buffer[ POS_TERM_HEADER ] << "). Data is corrupted or header info has not yet filled. Exiting...");
    }



  }

  inline void RawHeader_v1::SetNwords(int nwords)
  {
    CheckSetBuffer();
    m_buffer[ POS_NWORDS ] = nwords;

  }

  inline void RawHeader_v1::SetEveNo(unsigned int eve_no)
  {
    CheckSetBuffer();
    m_buffer[ POS_EVE_NO ] = eve_no;
  }

  inline void RawHeader_v1::SetNodeID(unsigned int node_id)
  {
    CheckSetBuffer();
    m_buffer[ POS_NODE_ID ] = (int)node_id;
  }

  inline void RawHeader_v1::SetDataType(int data_type)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
      (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
  }

  inline void RawHeader_v1::SetTruncMask(int trunc_mask)
  {
    CheckSetBuffer();
    /* cppcheck-suppress shiftTooManyBitsSigned */
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  /*   inline void RawHeader_v1::SetB2LFEEHdrPart(unsigned int word1, unsigned int word2) */
  /*   { */
  /*     m_buffer[ POS_HSLB_1 ] = word1; */
  /*     m_buffer[ POS_HSLB_2 ] = word2; */
  /*   } */


  inline void RawHeader_v1::SetOffset1stFINESSE(int offset_1st_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_1ST_FINESSE ] = offset_1st_FINESSE;
  }

  inline void RawHeader_v1::SetOffset2ndFINESSE(int offset_2nd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_2ND_FINESSE ] = offset_2nd_FINESSE;
  }

  inline void RawHeader_v1::SetOffset3rdFINESSE(int offset_3rd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_3RD_FINESSE ] = offset_3rd_FINESSE;
  }

  inline void RawHeader_v1::SetOffset4thFINESSE(int offset_4th_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_4TH_FINESSE ] = offset_4th_FINESSE;
  }

  /*   inline void RawHeader_v1::SetFTSW2Words(int* ftsw_buf) */
  /*   { */
  /*     CheckSetBuffer(); */
  /*     memcpy(&(m_buffer[ POS_HSLB_1 ]), (char*)ftsw_buf, sizeof(int) * 2); */
  /*     return; */
  /*   } */

  inline void RawHeader_v1::SetFTSW2Words(unsigned int word1,
                                          unsigned int word2)
  {
    CheckSetBuffer();
    m_buffer[ POS_TTCTIME_TRGTYPE ] = word1;
    m_buffer[ POS_TTUTIME ] = word2;
    return;
  }


  inline void RawHeader_v1::SetExpRunNumber(int* exprun_buf)
  {
    CheckSetBuffer();
    memcpy(&(m_buffer[ POS_EXP_RUN_NO ]), (char*)exprun_buf, sizeof(int) * 1);
    return;
  }


//
// Obtain info
//

  inline int RawHeader_v1::GetNwords()
  {
    CheckGetBuffer();
    return m_buffer[ POS_NWORDS ];
  }

  inline int RawHeader_v1::GetHdrNwords()
  {

    //    CheckGetBuffer();
    //    return m_buffer[ POS_HDR_NWORDS ];
    return RAWHEADER_NWORDS;
  }

  inline int RawHeader_v1::GetExpNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

  inline int RawHeader_v1::GetRunNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawHeader_v1::GetSubRunNo()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline unsigned int RawHeader_v1::GetExpRunSubrun()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]));
  }


  inline unsigned int RawHeader_v1::GetEveNo()
  {
    CheckGetBuffer();
    return m_buffer[ POS_EVE_NO ];
  }

  inline unsigned int RawHeader_v1::GetNodeID()
  {
    CheckGetBuffer();
    return (unsigned int)m_buffer[ POS_NODE_ID ];
  }

  inline int RawHeader_v1::GetDataType()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline int RawHeader_v1::GetTruncMask()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
  }

  inline unsigned int RawHeader_v1::GetErrorBitFlag()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TRUNC_MASK_DATATYPE ]);
  }

  inline void RawHeader_v1::AddErrorBitFlag(unsigned int error_bit_flag)
  {
    CheckGetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] |= (int)error_bit_flag;
    return;
  }

  inline int RawHeader_v1::GetOffset1stFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_1ST_FINESSE ];
  }

  inline int RawHeader_v1::GetOffset2ndFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_2ND_FINESSE ];
  }

  inline int RawHeader_v1::GetOffset3rdFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_3RD_FINESSE ];
  }

  inline int RawHeader_v1::GetOffset4thFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_4TH_FINESSE ];
  }

  /*   inline int RawHeader_v1::GetNumNodes() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_NUM_NODES ]; */
  /*   } */

  inline unsigned int RawHeader_v1::GetTTCtimeTRGType()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTCTIME_TRGTYPE ]);
  }

  inline int RawHeader_v1::GetTTCtime()
  {
    CheckGetBuffer();
    return (int)((GetTTCtimeTRGType() & TTCTIME_MASK) >> TTCTIME_SHIFT);
  }

  inline int RawHeader_v1::GetTRGType()
  {
    CheckGetBuffer();
    return (int)(GetTTCtimeTRGType() & TRGTYPE_MASK);
  }

  inline unsigned int RawHeader_v1::GetTTUtime()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTUTIME ]);
  }

  inline void RawHeader_v1::GetTTTimeVal(struct timeval* tv)
  {
    tv->tv_sec = GetTTUtime();
    tv->tv_usec = (int)(((double)GetTTCtime()) / 127.216);
    return ;
  }


  /*   inline unsigned int RawHeader_v1::GetMagicWordEntireHeader() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_TERM_HEADER ]; */
  /*   } */





}

#endif
