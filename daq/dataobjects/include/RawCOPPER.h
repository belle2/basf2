//+
// File : RawCOPPER.h
// Description : Base class to manage raw data element
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef RAWCOPPER_H
#define RAWCOPPER_H

// Includes
#include <daq/dataobjects/RawHeader.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawCOPPER : public TObject {
  public:
    //! Default constructor
    RawCOPPER();
    //! Constructor using existing pointer to raw data buffer
    RawCOPPER(int*);
    //! Destructor
    virtual ~RawCOPPER();

    //! copy rawdata into internal buffer
    virtual void copy(int*);

    //! get size of buffer
    virtual int size();

    //! allocate buffer
    virtual int* allocate_buffer(int nwords);

    //! get buffer
    virtual int* buffer();
    //! set buffer
    virtual void buffer(int*);

    //! get header
    virtual int* header(void);
    //! set header
    virtual void header(int*);

    //! get data
    virtual int* data(void);
    //! set
    virtual void data(int nwords, int*);

  private:
    int m_nwords;
    int* m_buffer; //[m_nwords]
    //    RawHeader m_header; //|| do not split the header
    bool m_allocated;

    ClassDef(RawCOPPER, 1);
  };
}

#endif
