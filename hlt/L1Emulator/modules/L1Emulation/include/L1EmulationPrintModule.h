/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L1EMULATIONPRINTMODULE_H
#define L1EMULATIONPRINTMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>


namespace Belle2 {
  /**
   * physics trigger
   */

  class L1EmulationPrintModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    L1EmulationPrintModule();

    /** Destructor */
    virtual ~L1EmulationPrintModule();

    /** Initialize the parameters */
    virtual void initialize();


    /** Event processor. */
    virtual void event();

    /** Termination action. */
    virtual void terminate();

  private:
    /**the sum of the weight of the processed events*/
    double m_weightcount;

    /**the number of trigger pathes*/
    int m_ntrg;

    /**the name of the trigger path*/
    std::string m_trgname[50];

    /**nEvent*/
    int m_nEvent;

    /**total trigger trsults*/
    double m_summary[50];

    /**trigger result of the event trigger by one path only*/
    double m_summaryonly[50];

  };
}

#endif /* L1EMULATIONPRINTMODULE_H */
