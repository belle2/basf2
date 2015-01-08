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
#include <daq/rawdata/modules/DAQConsts.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawFTSW.h>

/* Obsolete
#define DETECTOR_MASK 0xFFFFFFFF // tentative
#define SVD_ID 0x00000001 // tentative
#define CDC_ID 0x00000002 // tentative
#define TOP_ID 0x00000003 // tentative
#define ARICH_ID 0x00000004 // tentative
#define ECL_ID 0x00000005 // tentative
#define KLM_ID 0x00000006 // tentative
*/

namespace Belle2 {



  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerHLTModule : public DeSerializerPCModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerHLTModule();
    virtual ~DeSerializerHLTModule();

    //! initialize functions
    virtual void initialize();

    //! Module functions to be called from event process
    //    void FillNewRawCOPPERHeader(RawCOPPER* raw_copper);
    //! Module functions to be called from event process
    virtual void event();


  protected :
    StoreArray<RawCOPPER> rawcprarray;
    StoreArray<RawCDC> raw_cdcarray;
    StoreArray<RawSVD> raw_svdarray;
    StoreArray<RawTOP> raw_bpidarray;
    StoreArray<RawECL> raw_eclarray;
    StoreArray<RawARICH> raw_epidarray;
    StoreArray<RawKLM> raw_klmarray;
    StoreArray<RawFTSW> raw_ftswarray;

  private:
    // Parallel processing parameters





  };

} // end namespace Belle2

#endif // MODULEHELLO_H
