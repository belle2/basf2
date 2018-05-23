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
#include <vector>
#include <fstream>

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

  };
} // Namaspace Belle2
