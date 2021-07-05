/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZERFILE_H
#define DESERIALIZERFILE_H

#include <daq/rawdata/modules/DeSerializerCOPPER.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerFILEModule : public DeSerializerCOPPERModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerFILEModule();
    virtual ~DeSerializerFILEModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //! open a file
    virtual void fileOpen();

    //! receive data
    virtual int* readOneDataBlock(int* delete_flag, int* m_size_word, int* data_type);

    //! read data from file
    virtual int* readfromFILE(FILE* fp_in, const int size_word, const int start_word, const int stop_word);


  protected :

    enum {
      RAW_DATABLOCK = 0,  // RawDataBlock
      COPPER_DATABLOCK = 1 // Data from COPPER ( the 1st word should be 0x7fff0008 )
    };

    //!
    FILE* m_fp_in;

    StoreArray<RawDataBlock> m_rawDataBlock; /**< RawDataBlock. */
    StoreArray<RawCOPPER> m_rawCOPPER; /**< RawCOPPER. */
    StoreArray<RawCDC> m_rawCDC; /**< RawCDC. */
    StoreArray<RawSVD> m_rawSVD; /**< RawSVD. */
    StoreArray<RawTOP> m_rawTOP; /**< RawTOP. */
    StoreArray<RawECL> m_rawECL; /**< RawECL. */
    StoreArray<RawARICH> m_rawARICH; /**< RawARICH. */
    StoreArray<RawKLM> m_rawKLM; /**< RawKLM. */

    //!
    int* m_prev_buf;

    //!
    int m_prev_buf_flag;

    //!
    int m_dummy_evenum;

    //!
    int m_repetition_max;

    //!
    int m_repetition_cnt;

    //!
    std::string m_fname_in;

    //! a class to handle data from a file
    PostRawCOPPERFormat_latest m_tmp_header;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
