//+
// File : RawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWTRAILER_H
#define RAWTRAILER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>

//#define TRAILER_SIZE 16


/* #define   POS_CHKSUM 0 */
/* #define        POS_TERM_WORD 1 */

/* #define        RAWCOPPER_TRAILER_SIZE 2 */
/* #define        MAGIC_WORD_TERM_TRAILER 0x7fff0006 */


namespace Belle2 {

  //  class RawTrailer{
  class RawTrailer : public TObject {
  public:
    //! Default constructor
    RawTrailer();

    //! Constructor using existing pointer to raw data buffer
    RawTrailer(unsigned int*);

    //! Destructor
    ~RawTrailer();

    unsigned int* trailer(); //! return buffer

    void trailer(unsigned int* bufin); //! set buffer

    void initialize(); //! initialize header

    void set_chksum(int chksum);   //! initialize header

    void set_magic_word(); //!

    int get_trl_nwords(); //!

    unsigned int get_magic_word(); //!

  private:
    enum {
      RAWCOPPER_TRAILER_SIZE = 2,
      MAGIC_WORD_TERM_TRAILER = 0x7fff0006
    };

    enum {
      POS_CHKSUM = 0,
      POS_TERM_WORD = 1
    };
    unsigned int m_trailer[RAWCOPPER_TRAILER_SIZE];
    ClassDef(RawTrailer, 1);
  };
}

#endif
