/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua LI                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef L1EMULATIONMODULE_H
#define L1EMULATIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <string>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>
#include <hlt/L1Emulator/dataobjects/L1EmulationInformation.h>


namespace Belle2 {
  /**
   * physics trigger
   */

  class L1EmulationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    L1EmulationModule();

    /** Destructor */
    virtual ~L1EmulationModule();

    /** Initialize the parameters */
    virtual void initialize();


    /** Event processor. */
    virtual void event();

    /** Termination action. */
    virtual void terminate();

    /**Perform the selection criteria on event*/
    bool eventSelect();

    /**make a scale for a trigger path*/
    bool makeScale(int);

    /**make a scale as a function of the e-'s theta in lab*/
    bool makeScalefx(std::vector<int>);

  private:
    StoreArray<L1EmulationInformation> m_l1EmulationInformation; /** array for L1EmulationInformation */

    /**the users' cut*/
    std::string m_userCut;

    /**Access users' cut*/
    std::unique_ptr<Variable::Cut> m_cut;

    /**Scalefactor as a function of theta for Bhabha*/
    std::vector<int> m_scalefactor;

    /**the name of trigger path*/
    std::vector<std::string> trgname;

    /**the index of events*/
    int nEvent;

    /**the index of trigger path*/
    int m_ntrg;
  };
}

#endif /* L1EMULATIONMODULE_H */
