//+
// File : ReducedRawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef REDUCEDRAWTRAILER_H
#define REDUCEDRAWTRAILER_H

#include <string.h>

#include <stdlib.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>

//#define TRAILER_SIZE 16


/* #define   POS_CHKSUM 0 */
/* #define        POS_TERM_WORD 1 */

/* #define        RAWCOPPER_TRAILER_SIZE 2 */
/* #define        MAGIC_WORD_TERM_TRAILER 0x7fff0006 */


namespace Belle2 {
  /** \addtogroup dataobjects
    * @{
    */

  /**
   * The Raw Trailer class
   * This class defines the format of the trailer of RawCOPPER class data
   * and used for extracting trailer info from RawCOPPER object
   */
  class ReducedRawTrailer : public TObject {
  public:
    //! Default constructor
    ReducedRawTrailer();

    //! Destructor
    ~ReducedRawTrailer();

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

    int* m_buffer; //! do not record buffer ( RawCOPPER includes buffer of RawHeader and ReducedRawTrailer )
    //    unsigned int m_trailer[RAWCOPPER_TRAILER_SIZE];


    ClassDef(ReducedRawTrailer, 1);
  };


  inline void ReducedRawTrailer::CheckBuffer()
  {
    if (m_buffer == NULL) {
      perror("m_buffer is NULL. Exiting...");
      exit(1);
    }
  }

  inline int* ReducedRawTrailer::GetBuffer()
  {
    return m_buffer;
  }

  inline void ReducedRawTrailer::SetBuffer(int* bufin)
  {
    m_buffer = bufin;
  }

  inline void ReducedRawTrailer::Initialize()
  {
    SetMagicWord();
  }

  inline void ReducedRawTrailer::SetChksum(int chksum)
  {
    CheckBuffer();
    m_buffer[ POS_CHKSUM ] = chksum;
  }

  inline void ReducedRawTrailer::SetMagicWord()
  {
    CheckBuffer();
    m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
  }

  inline int ReducedRawTrailer::GetMagicWord()
  {
    CheckBuffer();
    return m_buffer[ POS_TERM_WORD ];
  }

  inline unsigned int ReducedRawTrailer::GetChksum()
  {
    CheckBuffer();
    return (unsigned int)(m_buffer[ POS_CHKSUM ]);
  }

  inline int ReducedRawTrailer::GetTrlNwords()
  {
    return RAWTRAILER_NWORDS;
  }




}

#endif
