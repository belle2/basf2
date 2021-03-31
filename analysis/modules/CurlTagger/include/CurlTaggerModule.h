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
#include <analysis/dataobjects/Particle.h>

#include <analysis/modules/CurlTagger/Selector.h>

namespace Belle2 {
  /**
  *This module is designed to tag curl tracks. Low pT particle will curl around in the barrel and be reconstructed as several tracks. This module will identify and tag these additional tracks for later removal.
  *
  */
  class CurlTaggerModule : public Module {

  public:

    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    CurlTaggerModule();

    /** destructor  */
    virtual ~CurlTaggerModule() override;

    /**  initialise */
    virtual void initialize() override;

    /**  begin run - unused*/
    virtual void beginRun() override;

    /** event code - all curl track selection done here */
    virtual void event() override;

    /** end run - unused */
    virtual void endRun() override;

    /** termination */
    virtual void terminate() override;


  private:
    /** preselects particles that may be curl tracks */
    bool passesPreSelection(Particle* particle);

    //params
    /** input particle lists */
    std::vector<std::string> m_ParticleLists;

    /** flags if data/mc comes from belle or belle II */
    bool m_BelleFlag;

    /** if true also does some truth based matching and tags the particles with truthCurl info */
    bool m_McStatsFlag;

    /** switch between training and classifying */
    bool m_TrainFlag;

    /** preselection pt cut */
    double m_PtCut;

    /** min classifier response to consider a match*/
    double m_ResponseCut;

    /** name of selector function to use*/
    std::string m_SelectorType;

    //member objects
    /** contains the selector used */
    CurlTagger::Selector* m_Selector = 0;

  }; // CurlTaggerModule
} //namespace

