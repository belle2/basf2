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
    SendTrailer(int*);

    //! Destructor
    ~SendTrailer();

    int* GetBuffer(); //! return buffer

    void SetBuffer(int* bufin); //! set buffer

    void Initialize(); //! initialize header

    void SetChksum(int chksum);   //! initialize header

    void SetMagicWord(); //!

    int GetTrlNwords(); //!

    int GetMagicWord(); //!

    enum {
      SENDTRL_NWORDS = 2
    };
  private:

    enum {
      POS_CHKSUM = 0,
      POS_TERM_WORD = 1
    };

    enum {
      MAGIC_WORD_SEND_TRAILER = 0x7fff0007
    };

    int m_buffer[ SENDTRL_NWORDS ];
    ClassDef(SendTrailer, 1);
  };
}

#endif
