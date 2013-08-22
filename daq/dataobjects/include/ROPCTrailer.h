//+
// File : ROPCTrailer.cc
// Description : Module to handle ROPCTraeiler attached on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 16 - Aug - 2013
//-

#ifndef ROPCTRAILER_H
#define ROPCTRAILER_H

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

  //  class ROPCTrailer{
  class ROPCTrailer : public TObject {
  public:
    //! Default constructor
    ROPCTrailer();

    //! Destructor
    ~ROPCTrailer();

    void CheckBuffer(); //! set buffer

    int* GetBuffer(); //! return buffer

    void SetBuffer(int* bufin); //! set buffer

    void Initialize(); //! set buffer

    void SetChksum(int chksum);   //! initialize header

    void SetMagicWord(); //!

    int GetTrlNwords(); //!

    int GetMagicWord(); //!

    enum {
      ROPCTRAILER_NWORDS = 2
    };

  private:
    enum {
      MAGIC_WORD_TERM_TRAILER = 0x7fff0007
    };

    enum {
      POS_CHKSUM = 0,
      POS_TERM_WORD = 1
    };

    //    unsigned int m_trailer[RAWCOPPER_TRAILER_SIZE];
    int* m_buffer; //! do not record buffer ( RawROPC includes buffer of RawHeader and RawTrailer )

    ClassDef(ROPCTrailer, 1);

  };
}

#endif
