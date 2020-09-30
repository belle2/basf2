//+
// File : RawTrailer_v2.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWTRAILER_V2_H
#define RAWTRAILER_V2_H

#include <stdio.h>

#include <rawdata/switch_basf2_standalone.h>

//#define TRAILER_SIZE 16
/* #define   POS_CHKSUM 0 */
/* #define        POS_TERM_WORD 1 */
/* #define        RAWCOPPER_TRAILER_SIZE 2 */
/* #define        MAGIC_WORD_TERM_TRAILER 0x7fff0006 */

namespace Belle2 {

  /**
   * The Raw Trailer class ver.1 ( the latest version since May, 2014 )
   * This class defines the format of the trailer of RawCOPPER class data
   * and used for extracting trailer info from RawCOPPER object
   */
  //  class RawTrailer_v2 : public TObject {
  class RawTrailer_v2 {
  public:
    //! Default constructor
    RawTrailer_v2();

    //! Destructor
    ~RawTrailer_v2();

    //! set buffer
    void CheckBuffer(); //! set buffer

    int* GetBuffer(); //! return buffer

    void SetBuffer(int* bufin); //! set buffer

    void Initialize(); //! set buffer

    void SetChksum(int chksum);   //! initialize header

    void SetMagicWord(); //! Set magic word

    int GetTrlNwords(); //! Set # of trailer words

    unsigned int GetChksum();   //! initialize header

    int GetMagicWord(); //! Get magic word

    enum {
      RAWTRAILER_NWORDS = 2
    };

    enum {
      MAGIC_WORD_TERM_TRAILER = 0x7fff0006
    };

    enum {
      POS_CHKSUM = 0,
      POS_TERM_WORD = 1
    };

  private:
    //! do not record buffer ( RawCOPPER includes buffer of RawHeader and RawTrailer_v2 )
    int* m_buffer; //! do not record

    /// To derive from TObject
    // ver.2 Do not record m_buffer pointer. (Dec.19, 2014)
    //    ClassDef(RawTrailer_v2, 1);
  };


  inline void RawTrailer_v2::CheckBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Exiting...");
    }
  }

  inline int* RawTrailer_v2::GetBuffer()
  {
    return m_buffer;
  }

  inline void RawTrailer_v2::SetBuffer(int* bufin)
  {
    m_buffer = bufin;
  }

  inline void RawTrailer_v2::Initialize()
  {
    SetMagicWord();
  }

  inline void RawTrailer_v2::SetChksum(int chksum)
  {
    CheckBuffer();
    m_buffer[ POS_CHKSUM ] = chksum;
  }

  inline void RawTrailer_v2::SetMagicWord()
  {
    CheckBuffer();
    m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
  }

  inline int RawTrailer_v2::GetMagicWord()
  {
    CheckBuffer();
    return m_buffer[ POS_TERM_WORD ];
  }

  inline unsigned int RawTrailer_v2::GetChksum()
  {
    CheckBuffer();
    return (unsigned int)(m_buffer[ POS_CHKSUM ]);
  }

  inline int RawTrailer_v2::GetTrlNwords()
  {
    return RAWTRAILER_NWORDS;
  }




}

#endif
