//+
// File : ReducedRawHeader.h
// Description : Module to handle ReducedRawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 5 - Mar - 2014
//-

#ifndef REDUCEDRAWHEADER_H
#define REDUCEDRAWHEADER_H

#include <string.h>
#include <stdlib.h>
#include <TObject.h>

#include <framework/datastore/DataStore.h>


//#define HEADER_SIZE 16

namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
   * The Raw Header class
   * This class defines the format of the header of RawCOPPER class data
   * and used for extracting header info from RawCOPPER object
   */

  class ReducedRawHeader : public TObject {
  public:
    //! Default constructor
    ReducedRawHeader();

    //! Constructor using existing pointer to raw data buffer
    ReducedRawHeader(int*);

    //! Destructor
    ~ReducedRawHeader();

    //! Get header contents
    int* GetBuffer() { return m_buffer; }

    //! set buffer
    void SetBuffer(int* bufin) { m_buffer = bufin; }

    //! initialize header
    void CheckSetBuffer();

    //! check if m_buffer exists
    void CheckGetBuffer();

    //! initialize header
    void Initialize();

    //! set contents of header
    void SetNwords(int nwords);

    void SetEveNo(unsigned int eve_no);    //! set contents of header

    void SetSubsysId(int subsys_id);    //! set contents of header

    void SetDataType(int data_type);    //! set contents of header

    void SetTruncMask(int trunc_mask);    //! set contents of header

    //    void SetB2LFEEHdrPart(unsigned int word1, unsigned int word2);   //! set contents of header
    //     void SetFTSW2Words(int* ftsw_buf);

    // Set values of FTSW info( trigger timing)
    void SetFTSW2Words(unsigned int word1, unsigned int word2);

    // Set a word consists of exp #, run # and subrun #
    void SetExpRunNumber(int* exprun_buf);

    void SetOffset1stFINESSE(int offset_1st_FINESSE);    //! set contents of header

    void SetOffset2ndFINESSE(int offset_2nd_FINESSE);    //! set contents of header

    void SetOffset3rdFINESSE(int offset_3rd_FINESSE);    //! set contents of header

    void SetOffset4thFINESSE(int offset_4th_FINESSE);    //! set contents of header

    //    void SetMagicWordEntireHeader(); //! set magic words;

    // Add nodeinfo in trace area
    int AddNodeInfo(int node_id);

    int GetNwords();  //! get contents of header

    int GetHdrNwords();  //! get contents of header

    unsigned int GetExpRunNumberWord(); //! get a run/exp number combined word

    int GetExpNo();  //! get contents of header

    int GetRunNoSubRunNo();    //! run# (14bit) restart # (8bit)

    int GetRunNo();    //! get run # (14bit)

    int GetSubRunNo();    //! get restart #(8bit)

    unsigned int GetEveNo();  //! get contents of header

    int GetSubsysId();  //! get contents of header

    int GetDataType();  //! get contents of header

    int GetTruncMask();  //! get contents of header

    int GetOffset1stFINESSE();  //! get contents of header

    int GetOffset2ndFINESSE();  //! get contents of header

    int GetOffset3rdFINESSE();  //! get contents of header

    int GetOffset4thFINESSE();  //! get contents of header

    //    int GetNumNodes();  //! get contents of header

    //    int GetNodeInfo(int node_no, int* node_id);    //! get contents of header

    unsigned int GetTTCtimeTRGType();  //! get contents of header

    int GetTTCtime();  //! get contents of header

    unsigned int GetTTUtime();  //! get contents of header

    void GetTTTimeVal(struct  timeval* tv);    //! get contents of header

    // Get magic word to check the data corruption
    //    unsigned int GetMagicWordEntireHeader();

    /*
      Experimental(10bit) #, Run#(14bit), restat# bit mask(8bit)
    */
    enum {
      EXP_MASK = 0xFFC00000,
      EXP_SHIFT = 22,
      RUNNO_MASK = 0x003FFF00,
      RUNNO_SHIFT = 8,
      SUBRUNNO_MASK = 0x000000FF
    };

    enum {
      MAGIC_MASK = 0xFFFF0000,
      MAGIC_SHIFT = 16,
      FORMAT_VERSION__MASK = 0x0000FF00,
      FORMAT_VERSION_SHIFT = 8,
      HDR_NWORDS_MASK = 0x000000FF
    };

    enum {
      RAWHEADER_NWORDS = 12
    };

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_TTCTIME_TRGTYPE = 4,
      POS_TTUTIME = 5,
      POS_SUBSYS_ID = 6,
      POS_TRUNC_MASK_DATATYPE = 7,
      POS_OFFSET_1ST_FINESSE = 8,
      POS_OFFSET_2ND_FINESSE = 9,
      POS_OFFSET_3RD_FINESSE = 10,
      POS_OFFSET_4TH_FINESSE = 11
                               /*       POS_RESVD_1 = 12, */
                               /*       POS_RESVD_2 = 13, */
                               /*       POS_NUM_NODES = 14, */
                               /*       POS_NODES_1 = 15, */
                               /*       POS_NODES_2 = 16, */
                               /*       POS_NODES_3 = 17, */
                               /*       POS_NODES_4 = 18, */
                               /*       POS_TERM_HEADER = 19 */
    };
    /* Data Format : Node info */
    /*     enum { */
    /*       NUM_MAX_NODES = 4   /\* (NUM_MAX_NODES + 1) slots are available in m_buffer. */
    /*          (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes */
    /*          exceeds NUM_MAX_NODES *\/ */
    /*     }; */

    /* Data Format : Magic word */
    /*     enum { */
    /*       MAGIC_WORD_TERM_HEADER = 0x7fff0005 */
    /*     }; */

    /* To extract ctime */
    enum {
      TTCTIME_MASK = 0x7FFFFFF0,
      TTCTIME_SHIFT = 4
    };

  private:

    int* m_buffer; //! do not record buffer ( RawCOPPER includes buffer of ReducedRawHeader and RawTrailer )
    //    unsigned int m_header[HEADER_SIZE];

    ClassDef(ReducedRawHeader, 1);
  };


  inline void ReducedRawHeader::CheckSetBuffer()
  {
    if (m_buffer == NULL) {
      perror("m_buffer is NULL. Exiting...");
      exit(1);
    }
  }

  inline void ReducedRawHeader::CheckGetBuffer()
  {
    //    if (m_buffer == NULL || m_buffer[ POS_TERM_HEADER ] != MAGIC_WORD_TERM_HEADER) {
    if (m_buffer == NULL) {
      printf("[DEBUG] m_buffer is NULL(%p). Data is corrupted or header info has not yet filled. Exiting...: %s %s %d", m_buffer, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      /*       printf("[DEBUG] m_buffer is NULL(%p) or magic word is invalid(0x%x). Data is corrupted or header info has not yet filled. Exiting...", */
      /*              m_buffer, */
      /*              m_buffer[ POS_TERM_HEADER ] */
      /*             ); */
      exit(1);
    }
  }



//
// Set info.
//
  inline void ReducedRawHeader::Initialize()
  {
    CheckSetBuffer();
    memset(m_buffer, 0, sizeof(int)*RAWHEADER_NWORDS);
    m_buffer[ POS_HDR_NWORDS ] = RAWHEADER_NWORDS;
    //    m_buffer[ POS_NUM_NODES ] = 0;

  }

  /*   inline void ReducedRawHeader::SetMagicWordEntireHeader() */
  /*   { */
  /*     m_buffer[ POS_TERM_HEADER ] = MAGIC_WORD_TERM_HEADER; */
  /*   } */

  inline void ReducedRawHeader::SetNwords(int nwords)
  {
    CheckSetBuffer();
    m_buffer[ POS_NWORDS ] = nwords;

  }

  inline void ReducedRawHeader::SetEveNo(unsigned int eve_no)
  {
    CheckSetBuffer();
    m_buffer[ POS_EVE_NO ] = eve_no;
  }

  inline void ReducedRawHeader::SetSubsysId(int subsys_id)
  {
    CheckSetBuffer();
    m_buffer[ POS_SUBSYS_ID ] = subsys_id;
  }

  inline void ReducedRawHeader::SetDataType(int data_type)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
      (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
  }

  inline void ReducedRawHeader::SetTruncMask(int trunc_mask)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  /*   inline void ReducedRawHeader::SetB2LFEEHdrPart(unsigned int word1, unsigned int word2) */
  /*   { */
  /*     m_buffer[ POS_HSLB_1 ] = word1; */
  /*     m_buffer[ POS_HSLB_2 ] = word2; */
  /*   } */


  inline void ReducedRawHeader::SetOffset1stFINESSE(int offset_1st_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_1ST_FINESSE ] = offset_1st_FINESSE;
  }

  inline void ReducedRawHeader::SetOffset2ndFINESSE(int offset_2nd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_2ND_FINESSE ] = offset_2nd_FINESSE;
  }

  inline void ReducedRawHeader::SetOffset3rdFINESSE(int offset_3rd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_3RD_FINESSE ] = offset_3rd_FINESSE;
  }

  inline void ReducedRawHeader::SetOffset4thFINESSE(int offset_4th_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_4TH_FINESSE ] = offset_4th_FINESSE;
  }

  /*   inline void ReducedRawHeader::SetFTSW2Words(int* ftsw_buf) */
  /*   { */
  /*     CheckSetBuffer(); */
  /*     memcpy(&(m_buffer[ POS_HSLB_1 ]), (char*)ftsw_buf, sizeof(int) * 2); */
  /*     return; */
  /*   } */

  inline void ReducedRawHeader::SetFTSW2Words(unsigned int word1,
                                              unsigned int word2)
  {
    CheckSetBuffer();
    m_buffer[ POS_TTCTIME_TRGTYPE ] = word1;
    m_buffer[ POS_TTUTIME ] = word2;
    return;
  }


  inline void ReducedRawHeader::SetExpRunNumber(int* exprun_buf)
  {
    CheckSetBuffer();
    memcpy(&(m_buffer[ POS_EXP_RUN_NO ]), (char*)exprun_buf, sizeof(int) * 1);
    return;
  }


//
// Obtain info
//

  inline int ReducedRawHeader::GetNwords()
  {
    CheckGetBuffer();
    return m_buffer[ POS_NWORDS ];
  }

  inline int ReducedRawHeader::GetHdrNwords()
  {
    /*     CheckGetBuffer(); */
    /*     return m_buffer[ POS_HDR_NWORDS ]; */
    return RAWHEADER_NWORDS;
  }

  inline int ReducedRawHeader::GetExpNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

  inline int ReducedRawHeader::GetRunNoSubRunNo()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int ReducedRawHeader::GetRunNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int ReducedRawHeader::GetSubRunNo()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline unsigned int ReducedRawHeader::GetExpRunNumberWord()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]));
  }


  inline unsigned int ReducedRawHeader::GetEveNo()
  {
    CheckGetBuffer();
    return m_buffer[ POS_EVE_NO ];
  }

  inline int ReducedRawHeader::GetSubsysId()
  {
    CheckGetBuffer();
    return m_buffer[ POS_SUBSYS_ID ];
//   unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int crate = m_buffer[ POS_CRATE_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int slot  = m_buffer[ POS_SLOT_ID + tmp_header.RAWHEADER_NWORDS ];
//   return
//     ((subsys << 16) & 0xFFFF0000) |
//     ((crate << 8) & 0x0000FF00) |
//     (slot & 0x000000FF);

  }

  inline int ReducedRawHeader::GetDataType()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline int ReducedRawHeader::GetTruncMask()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
  }


  inline int ReducedRawHeader::GetOffset1stFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_1ST_FINESSE ];
  }

  inline int ReducedRawHeader::GetOffset2ndFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_2ND_FINESSE ];
  }

  inline int ReducedRawHeader::GetOffset3rdFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_3RD_FINESSE ];
  }

  inline int ReducedRawHeader::GetOffset4thFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_4TH_FINESSE ];
  }

  /*   inline int ReducedRawHeader::GetNumNodes() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_NUM_NODES ]; */
  /*   } */

  inline unsigned int ReducedRawHeader::GetTTCtimeTRGType()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTCTIME_TRGTYPE ]);
  }

  inline int ReducedRawHeader::GetTTCtime()
  {
    CheckGetBuffer();
    return (int)((GetTTCtimeTRGType() & TTCTIME_MASK) >> TTCTIME_SHIFT);
  }

  inline unsigned int ReducedRawHeader::GetTTUtime()
  {
    CheckGetBuffer();
    return (unsigned int)(m_buffer[ POS_TTUTIME ]);
  }

  inline void ReducedRawHeader::GetTTTimeVal(struct timeval* tv)
  {
    tv->tv_sec = GetTTUtime();
    tv->tv_usec = (int)(((double)GetTTCtime()) / 127.216);
    return ;
  }


  /*   inline unsigned int ReducedRawHeader::GetMagicWordEntireHeader() */
  /*   { */
  /*     CheckGetBuffer(); */
  /*     return m_buffer[ POS_TERM_HEADER ]; */
  /*   } */





}

#endif
