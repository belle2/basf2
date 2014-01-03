//+
// File : RawHeader.h
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWHEADER_H
#define RAWHEADER_H

#include <string.h>
#include <stdlib.h>
#include <TObject.h>

#include <framework/datastore/DataStore.h>


//#define HEADER_SIZE 16

namespace Belle2 {

  class RawHeader : public TObject {
  public:
    //! Default constructor
    RawHeader();

    //! Constructor using existing pointer to raw data buffer
    RawHeader(int*);
    //! Destructor
    ~RawHeader();

    //! Get header contents
    int* GetBuffer() { return m_buffer; }

    //! set buffer
    void SetBuffer(int* bufin) { m_buffer = bufin; }

    //! initialize header
    void CheckSetBuffer();

    //!
    void CheckGetBuffer();

    //! initialize header
    void Initialize();

    //! set contents of header
    void SetNwords(int nwords);

    void SetEveNo(unsigned int eve_no);    //! set contents of header

    void SetSubsysId(int subsys_id);    //! set contents of header

    void SetDataType(int data_type);    //! set contents of header

    void SetTruncMask(int trunc_mask);    //! set contents of header

    void SetB2LFEEHdrPart(unsigned int word1, unsigned int word2);   //! set contents of header

    void SetFTSW2Words(int* ftsw_buf);

    void SetFTSW2Words(unsigned int word1, unsigned int word2);

    void SetExpRunNumber(int* exprun_buf);

    void SetOffset1stFINESSE(int offset_1st_FINESSE);    //! set contents of header

    void SetOffset2ndFINESSE(int offset_2nd_FINESSE);    //! set contents of header

    void SetOffset3rdFINESSE(int offset_3rd_FINESSE);    //! set contents of header

    void SetOffset4thFINESSE(int offset_4th_FINESSE);    //! set contents of header

    void SetMagicWordEntireHeader(); //! set magic words;

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

    int GetNumNodes();  //! get contents of header

    int GetNodeInfo(int node_no, int* node_id);    //! get contents of header

    unsigned int GetMagicWordEntireHeader();

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
      RAWHEADER_NWORDS = 20
    };

    /* Data Format : Fixed length part*/
    enum {
      POS_NWORDS = 0,
      POS_HDR_NWORDS = 1,
      POS_EXP_RUN_NO = 2,
      POS_EVE_NO = 3,
      POS_HSLB_1 = 4,
      POS_HSLB_2 = 5,
      POS_SUBSYS_ID = 6,
      POS_TRUNC_MASK_DATATYPE = 7,
      POS_OFFSET_1ST_FINESSE = 8,
      POS_OFFSET_2ND_FINESSE = 9,
      POS_OFFSET_3RD_FINESSE = 10,
      POS_OFFSET_4TH_FINESSE = 11,
      POS_RESVD_1 = 12,
      POS_RESVD_2 = 13,
      POS_NUM_NODES = 14,
      POS_NODES_1 = 15,
      POS_NODES_2 = 16,
      POS_NODES_3 = 17,
      POS_NODES_4 = 18,
      POS_TERM_HEADER = 19
    };
    /* Data Format : Node info */
    enum {
      NUM_MAX_NODES = 4   /* (NUM_MAX_NODES + 1) slots are available in m_buffer.
         (NUM_MAX_NODES +1 )th slot is filled with -1, when the number of total nodes
         exceeds NUM_MAX_NODES */
    };

    /* Data Format : Magic word */
    enum {
      MAGIC_WORD_TERM_HEADER = 0x7fff0005
    };

  private:

    int* m_buffer; //! do not record buffer ( RawCOPPER includes buffer of RawHeader and RawTrailer )
    //    unsigned int m_header[HEADER_SIZE];

    ClassDef(RawHeader, 1);
  };


  inline void RawHeader::CheckSetBuffer()
  {
    if (m_buffer == NULL) {
      perror("m_buffer is NULL. Exiting...");
      exit(1);
    }
  }

  inline void RawHeader::CheckGetBuffer()
  {
    if (m_buffer == NULL || m_buffer[ POS_TERM_HEADER ] != MAGIC_WORD_TERM_HEADER) {
      printf("m_buffer is NULL(%p) or magic word is invalid(0x%x). Data is corrupted or header info has not yet filled. Exiting...",
             m_buffer,
             m_buffer[ POS_TERM_HEADER ]
            );
      exit(1);
    }
  }



//
// Set info.
//
  inline void RawHeader::Initialize()
  {
    CheckSetBuffer();
    memset(m_buffer, 0, sizeof(int)*RAWHEADER_NWORDS);
    m_buffer[ POS_HDR_NWORDS ] = RAWHEADER_NWORDS;
    m_buffer[ POS_NUM_NODES ] = 0;

  }

  inline void RawHeader::SetMagicWordEntireHeader()
  {
    m_buffer[ POS_TERM_HEADER ] = MAGIC_WORD_TERM_HEADER;
  }

  inline void RawHeader::SetNwords(int nwords)
  {
    CheckSetBuffer();
    m_buffer[ POS_NWORDS ] = nwords;

  }

  inline void RawHeader::SetEveNo(unsigned int eve_no)
  {
    CheckSetBuffer();
    m_buffer[ POS_EVE_NO ] = eve_no;
  }

  inline void RawHeader::SetSubsysId(int subsys_id)
  {
    CheckSetBuffer();
    m_buffer[ POS_SUBSYS_ID ] = subsys_id;
  }

  inline void RawHeader::SetDataType(int data_type)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] =
      (data_type & 0x7FFFFFFF) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x80000000);
  }

  inline void RawHeader::SetTruncMask(int trunc_mask)
  {
    CheckSetBuffer();
    m_buffer[ POS_TRUNC_MASK_DATATYPE ] = (trunc_mask << 31) | (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline void RawHeader::SetB2LFEEHdrPart(unsigned int word1, unsigned int word2)
  {
    m_buffer[ POS_HSLB_1 ] = word1;
    m_buffer[ POS_HSLB_2 ] = word2;
  }


  inline void RawHeader::SetOffset1stFINESSE(int offset_1st_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_1ST_FINESSE ] = offset_1st_FINESSE;
  }

  inline void RawHeader::SetOffset2ndFINESSE(int offset_2nd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_2ND_FINESSE ] = offset_2nd_FINESSE;
  }

  inline void RawHeader::SetOffset3rdFINESSE(int offset_3rd_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_3RD_FINESSE ] = offset_3rd_FINESSE;
  }

  inline void RawHeader::SetOffset4thFINESSE(int offset_4th_FINESSE)
  {
    CheckSetBuffer();
    m_buffer[ POS_OFFSET_4TH_FINESSE ] = offset_4th_FINESSE;
  }

  inline void RawHeader::SetFTSW2Words(int* ftsw_buf)
  {
    CheckSetBuffer();
    memcpy(&(m_buffer[ POS_HSLB_1 ]), (char*)ftsw_buf, sizeof(int) * 2);
    return;
  }

  inline void RawHeader::SetFTSW2Words(unsigned int word1, unsigned int word2)
  {
    CheckSetBuffer();
    m_buffer[ POS_HSLB_1 ] = word1;
    m_buffer[ POS_HSLB_2 ] = word2;
    return;
  }


  inline void RawHeader::SetExpRunNumber(int* exprun_buf)
  {
    CheckSetBuffer();
    memcpy(&(m_buffer[ POS_EXP_RUN_NO ]), (char*)exprun_buf, sizeof(int) * 1);
    return;
  }


//
// Obtain info
//

  inline int RawHeader::GetNwords()
  {
    CheckGetBuffer();
    return m_buffer[ POS_NWORDS ];
  }

  inline int RawHeader::GetHdrNwords()
  {
    CheckGetBuffer();
    return m_buffer[ POS_HDR_NWORDS ];
  }

  inline int RawHeader::GetExpNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & EXP_MASK)
            >> EXP_SHIFT);
  }

  inline int RawHeader::GetRunNoSubRunNo()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) &
            (RUNNO_MASK | SUBRUNNO_MASK));
  }

  inline int RawHeader::GetRunNo()
  {
    CheckGetBuffer();
    return (((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]) & RUNNO_MASK)
            >> RUNNO_SHIFT);
  }

  inline int RawHeader::GetSubRunNo()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_EXP_RUN_NO ] & SUBRUNNO_MASK);
  }

  inline unsigned int RawHeader::GetExpRunNumberWord()
  {
    CheckGetBuffer();
    return ((unsigned int)(m_buffer[ POS_EXP_RUN_NO ]));
  }


  inline unsigned int RawHeader::GetEveNo()
  {
    CheckGetBuffer();
    return m_buffer[ POS_EVE_NO ];
  }

  inline int RawHeader::GetSubsysId()
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

  inline int RawHeader::GetDataType()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] & 0x7FFFFFFF);
  }

  inline int RawHeader::GetTruncMask()
  {
    CheckGetBuffer();
    return (m_buffer[ POS_TRUNC_MASK_DATATYPE ] >> 23) & 0x1;
  }


  inline int RawHeader::GetOffset1stFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_1ST_FINESSE ];
  }

  inline int RawHeader::GetOffset2ndFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_2ND_FINESSE ];
  }

  inline int RawHeader::GetOffset3rdFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_3RD_FINESSE ];
  }

  inline int RawHeader::GetOffset4thFINESSE()
  {
    CheckGetBuffer();
    return m_buffer[ POS_OFFSET_4TH_FINESSE ];
  }

  inline int RawHeader::GetNumNodes()
  {
    CheckGetBuffer();
    return m_buffer[ POS_NUM_NODES ];
  }



  inline unsigned int RawHeader::GetMagicWordEntireHeader()
  {
    return m_buffer[ POS_TERM_HEADER ];
  }



}

#endif
