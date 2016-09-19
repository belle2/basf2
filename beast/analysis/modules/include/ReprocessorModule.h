/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REPROCESSORMODULE_H
#define REPROCESSORMODULE_H

#include <mdst/dataobjects/MCParticleGraph.h>
#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TMath.h>
#include <TF1.h>
#include <TVector3.h>


namespace Belle2 {
  //  namespace reprocessor {
  /**
   * Reprocessor Module
   *
   * Re-launch MC Particle
   *
   */
  class ReprocessorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ReprocessorModule();

    /**  */
    virtual ~ReprocessorModule();

    /**  */
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /**  */
    virtual void event();

    /**  */
    virtual void endRun();
    /**  */
    virtual void terminate();


  private:

    /** Set PDG*/
    int m_input_TPC_PDG;
    /** Set N times */
    int m_input_TPC_Ntimes;
    /** Set PDG*/
    int m_input_HE3_PDG;
    /** Set N times */
    int m_input_HE3_Ntimes;

  };

  //  }
}

#endif /* REPROCESSORMODULE_H */
