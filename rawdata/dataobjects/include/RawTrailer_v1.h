/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawTrailer_v1.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWTRAILER_V1_H
#define RAWTRAILER_V1_H

#include <stdio.h>
#include <rawdata/switch_basf2_standalone.h>

/* #include <framework/datastore/DataStore.h> */
/* #include <TObject.h> */

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
  //  class RawTrailer_v1 : public TObject {
  class RawTrailer_v1  {
  public:
    //! Default constructor
    RawTrailer_v1();

    //! Destructor
    ~RawTrailer_v1();

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
    //! do not record buffer ( RawCOPPER includes buffer of RawHeader and RawTrailer_v1 )
    int* m_buffer; //! do not record

    /// To derive from TObject
    // ver.2 Do not record m_buffer pointer. (Dec.19, 2014)
    //    ClassDef(RawTrailer_v1, 1);
  };


  inline void RawTrailer_v1::CheckBuffer()
  {
    if (m_buffer == NULL) {
      B2FATAL("m_buffer is NULL. Exiting...");
    }
  }

  inline int* RawTrailer_v1::GetBuffer()
  {
    return m_buffer;
  }

  inline void RawTrailer_v1::SetBuffer(int* bufin)
  {
    m_buffer = bufin;
  }

  inline void RawTrailer_v1::Initialize()
  {
    SetMagicWord();
  }

  inline void RawTrailer_v1::SetChksum(int chksum)
  {
    CheckBuffer();
    m_buffer[ POS_CHKSUM ] = chksum;
  }

  inline void RawTrailer_v1::SetMagicWord()
  {
    CheckBuffer();
    m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
  }

  inline int RawTrailer_v1::GetMagicWord()
  {
    CheckBuffer();
    return m_buffer[ POS_TERM_WORD ];
  }

  inline unsigned int RawTrailer_v1::GetChksum()
  {
    CheckBuffer();
    return (unsigned int)(m_buffer[ POS_CHKSUM ]);
  }

  inline int RawTrailer_v1::GetTrlNwords()
  {
    return RAWTRAILER_NWORDS;
  }




}

#endif
