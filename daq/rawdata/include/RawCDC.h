//+
// File : rawcdc.h
// Description : pseudo raw data of CDC for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef RAWCDC_H
#define RAWCDC_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawCDC : public TObject {
  public:
    //! Default constructor
    RawCDC();
    //! Constructor using existing pointer to raw data buffer
    RawCDC(int nwords, unsigned int*);
    //! Destructor
    ~RawCDC();

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

    ClassDef(RawCDC, 1);
  };
}

#endif
