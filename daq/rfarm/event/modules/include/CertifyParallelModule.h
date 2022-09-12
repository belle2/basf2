/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CERTIFYPARALLELMODULE_H
#define CERTIFYPARALLELMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class CertifyParallelModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    CertifyParallelModule();
    virtual ~CertifyParallelModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

  private:
    int m_nevent;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
