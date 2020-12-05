//+
// File : HexDataPacker.h
// Description : Module to store dummy data in RawCOPPER object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-

#ifndef HEXDATAPACKER_H
#define HEXDATAPACKER_H

#include <fstream>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>

using namespace std;

#define MAX_CPRBUF_WORDS 5000

namespace Belle2 {

  /*! Module to store dummy data in RawCOPPER object */

  class HexDataPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    HexDataPackerModule();
    virtual ~HexDataPackerModule();

    //! initialization
    virtual void initialize() override;

    //! event module
    virtual void event() override;


  protected :
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! Node ID
    //    int m_nodeid;

    //! event counter
    int n_basf2evt;

    //! Messaage handler
    //    MsgHandler* m_msghandler;

    /*     //! Compression Level */
    /*     int m_compressionLevel; */

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawCOPPER array
    StoreArray<RawCOPPER> raw_cprarray;

    //! RawSVD array
    StoreArray<RawSVD> raw_svdarray;

    //! RawCDC array
    StoreArray<RawCDC> raw_cdcarray;

    //! RawTOP array
    StoreArray<RawTOP> raw_bpidarray;

    //! RawARICH array
    StoreArray<RawARICH> raw_epidarray;

    //! RawECL array
    StoreArray<RawECL> raw_eclarray;

    //! RawKLM array
    StoreArray<RawKLM> raw_klmarray;

    //! RawARICH array
    StoreArray<RawTRG> raw_trgarray;

  private:
    //! input-file stream
    ifstream m_ifs;

    //! input-file name
    string m_fileName;
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
