//+
// File : CertifyParallelModule.h
// Description : Module to measure elapsed time
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 07 - May - 2012
//-

#ifndef CERTIFYPARALLELMODULE_H
#define CERTIFYPARALLELMODULE_H

#include <framework/core/Module.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <sys/time.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class CertifyParallelModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    CertifyParallelModule();
    virtual ~CertifyParallelModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    int m_nevent;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
