#ifndef SVD_SETMETA_MODULE_H
#define SVD_SETMETA_MODULE_H
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
    virtual ~svdSetMetaModule();

    //! module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:

    std::string    m_svdRawName;

  };
} // Namaspace Belle2

#endif /* SVD_SETMETA_MODULE_H */
