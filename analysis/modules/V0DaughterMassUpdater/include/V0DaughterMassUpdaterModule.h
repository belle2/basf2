/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Yefan Tao    ustctao@ufl.edu                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef V0DAUGHTERMASSUPDATERMODULE_H
#define V0DAUGHTERMASSUPDATERMODULE_H
#pragma once
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

  /**This module replaces the mass of two daughters of the selected V0 particles inside the given particleLists with masses of given pdgCode.*/

  class V0DaughterMassUpdaterModule : public Module {
  private:
    /**PDG code for V0's daughters*/
    int m_pdg_dau0, m_pdg_dau1;

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    V0DaughterMassUpdaterModule();

    /** Initialises the module. */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2

#endif // V0DAUGHTERMASSUPDATERMODULE_H

