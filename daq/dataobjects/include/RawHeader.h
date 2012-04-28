//+
// File : RawHeader.h
// Description : pseudo raw data header for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef RAWHEADER_H
#define RAWHEADER_H

#include <string.h>

#include <framework/datastore/DataStore.h>

#include <TObject.h>

#define HEADER_SIZE 16

namespace Belle2 {

  class RawHeader : public TObject {
  public:
    //! Default constructor
    RawHeader();
    //! Constructor using existing pointer to raw data buffer
    RawHeader(unsigned int*);
    //! Destructor
    ~RawHeader();

    //! Get header contents
    unsigned int* header(void);

    //! set buffer
    void header(unsigned int* hdr);

  private:
    unsigned int m_header[HEADER_SIZE];

    ClassDef(RawHeader, 1);
  };
}

#endif
