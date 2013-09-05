//+
// File : RawPXD.h
// Description : pseudo raw data of PXD for test
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Jul - 2013
//-

#ifndef RAWPXD_H
#define RAWPXD_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

namespace Belle2 {

  class RawPXD : public TObject {
  public:

    RawPXD();

    //! Constructor using existing pointer to raw data buffer
    RawPXD(int* , int);

    //! Destructor
    virtual ~RawPXD();

    //! get size of buffer in 32 Bit words
    virtual int size();
    /*at the moment not used
        //! allocate buffer
        virtual int* allocate_buffer(int nwords);

        //! set data size in 32 Bit words
        virtual void data(int nwords, int*);
    */
    //! get data
    virtual int* data(void);

  private:
    int m_nwords;
    int* m_buffer; //[m_nwords]
    bool m_allocated;

    ClassDef(RawPXD, 1)
  };
}

#endif
