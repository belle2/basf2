/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ELAPSEDTIMEMODULE_H
#define ELAPSEDTIMEMODULE_H

#include <framework/core/Module.h>

#include <sys/time.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class ElapsedTimeModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    ElapsedTimeModule();
    virtual ~ElapsedTimeModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    // Data members
  private:
    int m_nevent;
    int m_nevprev;
    int m_interval;
    struct timeval m_t0;
    struct timeval m_tprev;
    struct timeval m_tnow;
    struct timeval m_tend;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
