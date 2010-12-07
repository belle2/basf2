//+
// File : rawheader.h
// Description : pseudo raw data header for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef RAWHEADER_H
#define RAWHEADER_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawHeader : public TObject {
  public:
    //! Default constructor
    RawHeader();
    //! Constructor using existing pointer to raw data buffer
    RawHeader(int nwords, unsigned int*);
    //! Destructor
    ~RawHeader();

    //! copy rawdata into internal buffer
    void copy(int nwords, unsigned int*);

    //! allocate buffer
    unsigned int* allocate_buffer(int nwords);

    //! get buffer
    unsigned int* get_buffer();

    //! set buffer
    void set_buffer(int, unsigned int*);

  private:
    unsigned int* m_buffer;
    int m_nwords;
    bool m_allocated;

    ClassDef(RawHeader, 1);
  };
}

#endif
