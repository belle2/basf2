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

namespace Belle2 {

  /*! Module to store dummy data in RawCOPPER object */
  class HexDataPackerModule : public Module {

    // Public functions
  public:

    //! Constructor
    HexDataPackerModule();

    //! Destructor
    virtual ~HexDataPackerModule();

    //! initialization
    virtual void initialize() override;

    //! event module
    virtual void event() override;


  protected :
    //! event counter
    int n_basf2evt;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawCOPPER array
    StoreArray<RawCOPPER> m_raw_cprarray;

    //! RawSVD array
    StoreArray<RawSVD> m_raw_svdarray;

    //! RawCDC array
    StoreArray<RawCDC> m_raw_cdcarray;

    //! RawTOP array
    StoreArray<RawTOP> m_raw_bpidarray;

    //! RawARICH array
    StoreArray<RawARICH> m_raw_epidarray;

    //! RawECL array
    StoreArray<RawECL> m_raw_eclarray;

    //! RawKLM array
    StoreArray<RawKLM> m_raw_klmarray;

    //! RawARICH array
    StoreArray<RawTRG> m_raw_trgarray;

  private:
    //! input-file stream
    std::ifstream m_ifs;

    //! input-file name
    std::string m_fileName;
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
