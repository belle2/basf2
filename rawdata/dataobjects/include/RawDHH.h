//+
// File : RawDHH.h
// Description : pseudo raw data of DHH (BonnDAQ) for test
//
// Author : Bjoern Spruck
// Date : 07.4.2016
//-

#ifndef RAWDHH_H
#define RAWDHH_H

#include <framework/datastore/DataStore.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * The Raw DHH class.
   *
   * This class stores the RAW data received from the DHH system.
   * This data is then decoded by the pxdDHHUnpacker module.
   * For system simulation, pxdDHHPacker writes out in this objects.
   */
  class RawDHH : public TObject {
  public:

    //! Default constructor
    RawDHH();

    //! Constructor using existing pointer to raw data buffer which needs to be _copied_
    RawDHH(int*, int);

    //! Destructor
    virtual ~RawDHH();

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
    /// Raw dump of DHH data. buffer of size m_nwords (32bit int)
    int* m_buffer; //[m_nwords] /**< Buffer of size m_nwords  */

    ClassDef(RawDHH, 1)
  }; // class RawDHH


} // end namespace Belle2

#endif
