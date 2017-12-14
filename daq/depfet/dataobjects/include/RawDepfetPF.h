//+
// File : RawDepfetPF.h
// Description : pseudo raw data of PXD for test
//
// Author : Bjoern Spruck / Klemens Lautenbach
// Date : 13 - Jul - 2013
// Updated 17.7.2015
//-

#ifndef RAWDEPFETPF_H
#define RAWDEPFETPF_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * The Raw PXD class.
   *
   * This class stores the RAW data received from the ONSEN system.
   * This data is then decoded by the pxdUnpacker module.
   * For system simulation, pxdPacker writes out in this objects.
   */
  class RawDepfetPF : public TObject {
  public:

    //! Default constructor
    RawDepfetPF();

    //! Constructor using existing pointer to raw data buffer which needs to be _copied_
    RawDepfetPF(int*, int);

    //! Destructor
    virtual ~RawDepfetPF();

    //! get size of buffer in 32 Bit words
    virtual int size();
    /*at the moment not used
        //! allocate buffer
        virtual int* allocate_buffer(int nwords);

        //! set data size in 32 Bit words
        virtual void data(int nwords, int*);
    */
    //! get pointer to data
    virtual int* data(void);

  private:
    int m_nwords;/**< Number of (32bit) Words stored in the buffer. There might be unsused bytes in the last word.*/
    /// Raw dump of ONSEN data. buffer of size m_nwords (32bit int)
    int* m_buffer; //[m_nwords] /**< Buffer of size m_nwords  */

    //! Endian swap a int32
    unsigned int endian_swap(unsigned int x);

    ClassDef(RawDepfetPF, 1)
  }; // class RawDepfetPF


} // end namespace Belle2

#endif
