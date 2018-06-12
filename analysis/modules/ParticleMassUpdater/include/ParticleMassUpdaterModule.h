/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Fernando Abudinen                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef PARTICLEMASSUPDATERMODULE_H
#define PARTICLEMASSUPDATERMODULE_H
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TMatrix.h>

namespace Belle2 {

  /** This module replaces the mass of the particles inside the given particleLists
  with the invariant mass of the particle corresponding to the given pdgCode. */

  class ParticleMassUpdaterModule : public Module {
  private:
    /** PDG code for mass reference **/
    int m_pdgCode;
    /*flag to turn on the V0 daughters updating. If flase, particles in selected particle list will be updated. If true, daughters of particles in selected particle list will be updated*/
    bool m_updateDaughters;
    /*PDG code for V0's daughters*/
    int m_pdg_dau0, m_pdg_dau1;

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    ParticleMassUpdaterModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2

#endif // PARTICLEMASSUPDATERMODULE_H

