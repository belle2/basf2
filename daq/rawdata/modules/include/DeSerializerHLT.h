//+
// File : DeSerializerHLT.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESERIALIZERHLT_H
#define DESERIALIZERHLT_H
#include <daq/rawdata/modules/DeSerializerPC.h>
#include <daq/dataobjects/RawCDC.h>
#include <daq/rawdata/modules/DAQConsts.h>

#define DETECTOR_MASK 0xFFFFFFFF // tentative
#define CDC_ID 0x00000001 // tentative
#define SVD_ID 0x00000002 // tentative
#define ECL_ID 0x00000003 // tentative
#define BPID_ID 0x00000004 // tentative
#define EPID_ID 0x00000005 // tentative
#define KLM_ID 0x00000006 // tentative

namespace Belle2 {



  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerHLTModule : public DeSerializerPCModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerHLTModule();
    virtual ~DeSerializerHLTModule();

    //! Module functions to be called from event process
    //    void FillNewRawCOPPERHeader(RawCOPPER* raw_copper);

    //! Module functions to be called from event process
    virtual void event();


  protected :
    // Data members

    //!    StoreObjPtr<RawCOPPER> m_rawcopper;
    StoreArray<RawCDC> raw_cdcarray;
    /*     StoreArray<RawSVD> raw_cdcarray; */
    /*     StoreArray<RawECL> raw_cdcarray; */
    /*     StoreArray<RawBPID> raw_cdcarray; */
    /*     StoreArray<RawCDC> raw_cdcarray; */

  private:
    // Parallel processing parameters





  };

} // end namespace Belle2

#endif // MODULEHELLO_H
