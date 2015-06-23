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
#include <string>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>


namespace Belle2 {
  /**
   * physics trigger
   *
   *    *
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
    void eventSelect();
//  void eventInformation();



  private:



    /**Switch of customing the selection criteria by user.*/
    int m_userCustomOpen;
    /**the total number of events*/
    int Total_Event;
    /**the number of events triggered by TRGi*/
    int TRG_Event[50];
    /**the number of events triggered by TRGi only*/
    int TRG_Event_TRGOnly[50];
    /**the number of events triggered by TRGi and TRGj simultaneously */
    int TRG_Event_Matrix[50][50];
    /**the TRG*/
    Variable::Cut::Parameter m_userCut[50];


    /**Access users' cut*/
    Variable::Cut m_cut;

    /**Trigger results*/
    int m_summary[50];
    /**Trigger results that triggered by one TRGi only*/
    int m_summary_TRGonly[50];

  };
}

#endif /* PHYSICSTRIGGERMODULE_H */
