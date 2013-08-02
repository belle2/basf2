//+
// File : SendTrailer.h
// Description : Module to handle SendTrailer attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SENDTRAILER_H
#define SENDTRAILER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>


namespace Belle2 {

  //  class SendTrailer{
  class SendTrailer : public TObject {
  public:
    //! Default constructor
    SendTrailer();

    //! Constructor using existing pointer to raw data buffer
    SendTrailer(unsigned int*);

    //! Destructor
    ~SendTrailer();

    unsigned int* trailer(); //! return buffer

    void trailer(unsigned int* bufin); //! set buffer

    void initialize(); //! initialize header

    void set_chksum(int chksum);   //! initialize header

    void set_magic_word(); //!

    int get_trl_nwords(); //!

    unsigned int get_magic_word(); //!

  private:

    enum {
      POS_CHKSUM = 0,
      POS_TERM_WORD = 1
    };

    enum {
      NUM_TRAILER_WORDS = 2,
      MAGIC_WORD_SEND_TRAILER = 0x7fff0007
    };
    unsigned int m_trailer[ NUM_TRAILER_WORDS ];
    ClassDef(SendTrailer, 1);
  };
}

#endif
