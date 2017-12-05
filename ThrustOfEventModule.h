/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: iorch                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef THRUSTOFEVENTMODULE_H
#define THRUSTOFEVENTMODULE_H

#include <framework/core/Module.h>


namespace Belle2 {
  /**
   * Module to compute Thrust of a particle list, mainly used to compute the thrust of a tau-taubar event.
   *
   *    *
   */
  class ThrustOfEventModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ThrustOfEventModule();

    /** free memory */
    virtual ~ThrustOfEventModule();

    /** Define the physical parameters. llok for them in database. */
    virtual void initialize();

    /** Define run parameters. */
    virtual void beginRun();

    /** Define event parameters */
    virtual void event();

    /** Finish the run. */
    virtual void endRun();

    /** finish the execution  */
    virtual void terminate();


  private:

    v m_v m_pList;  /**< Particle List used to compute the Thrust */
  };
}

#endif /* THRUSTOFEVENTMODULE_H */
