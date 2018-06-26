#pragma once
//+
// File : svdSetMetaModule
// Description : A module to set Meta info from ftsw data
//
// Author : Bjoern Spruck
// Date : 17 - June - 2016
//-

#include <framework/core/Module.h>

#include <string>
#include <ctime>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {

  /*! Class definition of svdClsHistoManager module */
  class svdSetMetaModule : public Module {
  public:
    //! Constructor and Destructor
    svdSetMetaModule();

  private:
    //! module functions
    void initialize() override final;
    void event() override final;

    std::string    m_svdRawName;
    StoreArray<RawSVD> m_rawSVD;
    StoreObjPtr<EventMetaData> m_evtPtr;

  };
} // Namaspace Belle2
