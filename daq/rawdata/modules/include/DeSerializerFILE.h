//+
// File : DeSerializerPC.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESERIALIZERFILE_H
#define DESERIALIZERFILE_H

#include <daq/rawdata/modules/DeSerializerCOPPER.h>
#include <daq/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/RawCDC.h>
#include <daq/dataobjects/RawSVD.h>
#include <daq/dataobjects/RawECL.h>
#include <daq/dataobjects/RawEPID.h>
#include <daq/dataobjects/RawBPID.h>
#include <daq/dataobjects/RawKLM.h>
#include <daq/dataobjects/RawFTSW.h>
#include <daq/rawdata/modules/DAQConsts.h>



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

    //! receive data
    virtual int* ReadOneDataBlock(int* malloc_flag, int* m_size_word, int* data_type);

    //! read data from file
    virtual int* ReadfromFILE(FILE* fp_in, const int size_word, const int start_word, const int stop_word);



  protected :

    enum {
      RAW_DATABLOCK = 0,  // RawDataBlock
      COPPER_DATABLOCK = 1 // Data from COPPER ( the 1st word should be 0x7fff0008 )
    };

    //!
    FILE* m_fp_in;

    //    StoreArray<RawDataBlock> raw_dblkarray;
    /*     StoreArray<RawCOPPER> rawcprarray; */
    /*     StoreArray<RawCDC> raw_cdcarray; */
    /*     StoreArray<RawSVD> raw_svdarray; */
    /*     StoreArray<RawBPID> raw_bpidarray; */
    /*     StoreArray<RawECL> raw_eclarray; */
    /*     StoreArray<RawEPID> raw_epidarray; */
    /*     StoreArray<RawKLM> raw_klmarray; */
    /*     StoreArray<RawFTSW> raw_ftswarray; */

    //!
    int* m_prev_buf;

    //!
    int m_prev_buf_flag;

    //!
    ErrorMessage print_err;

    //!
    std::string m_fname_in;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
