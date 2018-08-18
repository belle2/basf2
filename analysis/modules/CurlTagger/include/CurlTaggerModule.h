/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/TrackVariables.h>

#include <iostream>
#include <vector>
#include <string>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"


//Module Includes
#include <analysis/modules/CurlTagger/Bundle.h>

#include <analysis/modules/CurlTagger/Selector.h>
#include <analysis/modules/CurlTagger/SelectorCut.h>

namespace Belle2 {
  /**
  *This module is designed to tag curl tracks. Low pT particle will curl around in the barrel and be reconstructed as several tracks.
  *This module will identify and tag these additional tracks for later removal.
  *
  */
  class CurlTaggerModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    CurlTaggerModule();

    /**  */
    virtual ~CurlTaggerModule() override;

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /**  */
    virtual void terminate() override;


  private:
    /** preselects particles that may be curl tracks */
    bool passesPreSelection(Particle* particle);

    //params
    std::vector<std::string> m_ParticleLists;  /**< input particle lists */
    bool m_BelleFlag; /**< flags if data/mc comes from belle or belle II */
    bool m_McStatsFlag; /**< if true outputs statistics */
    double m_PtCut; /**< preselection pt cut */
    double m_PVal; /**< min allowed PVal for a match */
    std::string m_SelectorType; /**< name of selector function */

    //member objects
    CurlTagger::Selector m_Selector;
  }; // CurlTaggerModule
} //namespace

