/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAWTRAILER_LATEST_H
#define RAWTRAILER_LATEST_H

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
  //  class RawTrailer_latest : public TObject {
  class RawTrailer_latest {
  public:
    //! Default constructor
    RawTrailer_latest();

    //! Destructor
    ~RawTrailer_latest();

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
      RAWTRAILER_NWORDS = 4
    };

    enum {
      MAGIC_WORD_TERM_TRAILER = 0x7fff0006
    };

    enum {
      POS_ERROR_BIT_CH1 = 0,
      POS_ERROR_CH2 = 1,
      POS_CHKSUM = 2,
      POS_TERM_WORD = 3
    };

  private:
    //! do not record buffer ( RawCOPPER includes buffer of RawHeader and RawTrailer_latest )
    int* m_buffer; //! do not record

    /// To derive from TObject
    // ver.2 Do not record m_buffer pointer. (Dec.19, 2014)
    //    ClassDef(RawTrailer_latest, 1);
  };


  inline void RawTrailer_latest::CheckBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Exiting...");
    }
  }

  inline int* RawTrailer_latest::GetBuffer()
  {
    return m_buffer;
  }

  inline void RawTrailer_latest::SetBuffer(int* bufin)
  {
    m_buffer = bufin;
  }

  inline void RawTrailer_latest::Initialize()
  {
    SetMagicWord();
  }

  inline void RawTrailer_latest::SetChksum(int chksum)
  {
    CheckBuffer();
    m_buffer[ POS_CHKSUM ] = chksum;
  }

  inline void RawTrailer_latest::SetMagicWord()
  {
    CheckBuffer();
    m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
  }

  inline int RawTrailer_latest::GetMagicWord()
  {
    CheckBuffer();
    return m_buffer[ POS_TERM_WORD ];
  }

  inline unsigned int RawTrailer_latest::GetChksum()
  {
    CheckBuffer();
    return (unsigned int)(m_buffer[ POS_CHKSUM ]);
  }

  inline int RawTrailer_latest::GetTrlNwords()
  {
    return RAWTRAILER_NWORDS;
  }




}

#endif
