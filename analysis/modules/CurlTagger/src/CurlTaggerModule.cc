/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/CurlTagger/CurlTaggerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <analysis/VariableManager/VertexVariables.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <analysis/VariableManager/Variables.h>

#include <iostream>
#include <vector>
#include <string>

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"

//Module Includes
#include <analysis/modules/CurlTagger/Bundle.h>
#include <analysis/modules/CurlTagger/SelectorCut.h>
#include <analysis/modules/CurlTagger/SelectorMVA.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CurlTagger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CurlTaggerModule::CurlTaggerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("This module is designed to tag curl tracks"
    )DOC");

  // Parameter definitions
  addParam("particleLists", m_ParticleLists, "input particle lists");
  addParam("belle", m_BelleFlag, "flag to distinuguish belle (true) from belle II (false) data", false);
  addParam("ptCut", m_PtCut, "preselection pt Cut", 0.4);
  addParam("selectorType", m_SelectorType,
           "gives the name of the selector to use, available : 'cut', 'mva'", std::string("cut"));
  addParam("mcTruth", m_McStatsFlag, "outputs extra stats based on MC truth", false);
  addParam("train", m_TrainFlag, "flag for training the MVA or other methods if needed", false);

  addParam("responseCut", m_ResponseCut, "min allowed selector response for a match", 0.342);
}

CurlTaggerModule::~CurlTaggerModule()
{
}

bool CurlTaggerModule::passesPreSelection(Particle* p)
{
  if (Variable::particlePt(p) > m_PtCut) {return false;}
  if (Variable::trackNCDCHits(p) == 0 && Variable::trackNVXDHits(p) == 0) {return false;} //should never happen anyway but might as well check
  if (p -> getCharge() == 0) {return false;}
  return true;
}

void CurlTaggerModule::initialize()
{
  //initialise the selection function chosen by user
  if (m_SelectorType.compare("cut") == 0) {
    m_Selector = new CurlTagger::SelectorCut(m_BelleFlag);
    //Only really works for belle data right now
    if (!m_BelleFlag) {
      B2WARNING("Curl Tagger 'cut' selector is only calibrated for Belle");
    }

  } else if (m_SelectorType.compare("mva") == 0) {
    m_Selector = new CurlTagger::SelectorMVA(m_BelleFlag, m_TrainFlag);
  } else {
    B2ERROR("Curl Track Tagger - Selector type does not exists.");
  }

  //initialse the selector if it has an initialize function
  m_Selector->initialize();
}

void CurlTaggerModule::beginRun()
{
}

void CurlTaggerModule::event()
{
  for (auto& iList : m_ParticleLists) {
    StoreObjPtr<ParticleList> particleList(iList);

    //check particle List exists and has particles
    if (!particleList) {
      B2ERROR("ParticleList " << iList << " not found");
      continue;
    }
    unsigned int particleListSize = particleList -> getListSize();
    if (particleListSize == 0) {
      continue;
    }

    // Classify
    if (!m_TrainFlag) {
      std::vector<CurlTagger::Bundle> bundles;
      std::vector<CurlTagger::Bundle> truthBundles; //only used if mcstatsFlag is true but empty lists are basically free

      for (unsigned int i = 0; i < particleListSize; i++) {

        Particle* iPart = particleList -> getParticle(i);
        iPart -> addExtraInfo("isCurl", 0);
        iPart -> addExtraInfo("bundleSize", 0);
        if (m_McStatsFlag) {
          iPart -> addExtraInfo("isTruthCurl", 0);
          iPart -> addExtraInfo("truthBundleSize", 0);
        }
        if (!passesPreSelection(iPart)) {continue;}

        bool addedParticleToBundle = false;
        std::vector<float> bundlesProb;

        for (CurlTagger::Bundle bundle : bundles) {
          unsigned int bundleSize = bundle.size();
          float averageResponse = 0;

          for (unsigned int b = 0; b < bundleSize; b++) {
            Particle* bPart = bundle.getParticle(b);
            averageResponse += m_Selector -> getResponse(iPart, bPart);
          }

          averageResponse /= bundleSize;
          bundlesProb.push_back(averageResponse);
        } //bundles

        if (bundlesProb.size() > 0) {
          auto maxElement = std::max_element(bundlesProb.begin(), bundlesProb.end());
          if (*maxElement > m_ResponseCut) {
            int maxPosition = std::distance(std::begin(bundlesProb), maxElement);
            bundles[maxPosition].addParticle(iPart);
            addedParticleToBundle = true;
          }
        }

        if (!addedParticleToBundle) {
          CurlTagger::Bundle tempBundle = CurlTagger::Bundle(false);
          tempBundle.addParticle(iPart);
          bundles.push_back(tempBundle);
        }

        if (m_McStatsFlag) {
          bool addedParticleToTruthBundle = false;
          for (unsigned int tb = 0; tb < truthBundles.size(); tb++) {
            Particle* bPart = truthBundles[tb].getParticle(0);
            if (Variable::genParticleIndex(iPart) == Variable::genParticleIndex(bPart)) {
              truthBundles[tb].addParticle(iPart);
              addedParticleToTruthBundle = true;
              break;
            } // same genParticleIndex
          } //truthBundles
          if (!addedParticleToTruthBundle) {
            CurlTagger::Bundle truthTempBundle = CurlTagger::Bundle(true);
            truthTempBundle.addParticle(iPart);
            truthBundles.push_back(truthTempBundle);
          } //create new truth bundle
        }//MCStatsFlag
      } // iParticle
      for (CurlTagger::Bundle bundle : bundles) {
        bundle.tagCurlInfo();
        if (m_McStatsFlag) {
          bundle.tagSizeInfo();
        }
      }
      if (m_McStatsFlag) {
        for (CurlTagger::Bundle truthBundle : truthBundles) {
          truthBundle.tagCurlInfo();
          truthBundle.tagSizeInfo();
        }
      }
    } else {// !TrainFlag
      for (unsigned int i = 0; i < particleListSize; i++) {
        Particle* iPart = particleList -> getParticle(i);
        if (!passesPreSelection(iPart)) {continue;}

        for (unsigned int j = 0; j < particleListSize; j++) {
          Particle* jPart = particleList -> getParticle(j);
          if (i == j) {continue;}
          if (!passesPreSelection(jPart)) {continue;}

          m_Selector->collectTrainingInfo(iPart, jPart);
        } //jPart
      } //iPart
    } // Training events
  } // particle Lists
}

void CurlTaggerModule::endRun()
{
}

void CurlTaggerModule::terminate()
{
  m_Selector->finalize();
}



