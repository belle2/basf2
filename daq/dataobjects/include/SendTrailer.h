/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SENDTRAILER_H
#define SENDTRAILER_H

namespace Belle2 {

  //  class SendTrailer{
  //  class SendTrailer : public TObject {
  class SendTrailer {
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
    //    ClassDef(SendTrailer, 1);
  };
}

#endif
