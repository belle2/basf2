#ifndef HISTO_MANAGER_H
#define HISTO_MANAGER_H
//+
// File : HistoManager.h
// Description : A module to manage histograms/ntuples/ttrees
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-

#include <framework/core/Module.h>
#include <framework/pcore/RbTuple.h>

#include <string>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   */

  /*! Class definition of HistoManager module */
  class HistoManagerModule : public Module {
  public:

    //! Constructor and Destructor
    HistoManagerModule();
    virtual ~HistoManagerModule();

    //! module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::string m_histfile;
    bool        m_initmain;
    bool        m_initialized;
  };

  /*! @} */
} // Namaspace Belle2

#endif /* HISTO_MANAGER_H */
