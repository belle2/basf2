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
#include <analysis/modules/CurlTagger/SelectorCut.h>


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
  addParam("belleFlag", m_BelleFlag, "flag to distinuguish belle (true) from belle II (false) data", true);
  addParam("ptCut", m_PtCut, "preselection pt Cut", 0.4);
  addParam("selectorType", m_SelectorType,
           "gives the name of the selector to use, currently only cut based ('cut') selection is available", std::string("cut"));
  addParam("mcStatsFlag", m_McStatsFlag, "outputs extra stats based on MC truth");
  addParam("pVal", m_PVal, "min allowed pVal for a match");
}

CurlTaggerModule::~CurlTaggerModule()
{
}

bool CurlTaggerModule::passesPreSelection(Particle* p)
{
  if (Variable::particlePt(p) > m_PtCut) {return false;}
  if (Variable::trackNCDCHits(p) == 0 && Variable::trackNVXDHits(p) == 0) {return false;}
  if (p -> getCharge() == 0) {return false;}
  return true;
}

void CurlTaggerModule::initialize()
{
  //initialise the selection function chosen by user
  if (m_SelectorType.compare("cut") == 0) {
    m_Selector = new CurlTagger::SelectorCut(m_BelleFlag);
  } else {
    B2ERROR("Curl Track Tagger - Selector type does not exists.");
  }
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
        float averageProb = 0;

        for (unsigned int b = 0; b < bundleSize; b++) {
          Particle* bPart = bundle.getParticle(b);
          averageProb += m_Selector -> getProbability(iPart, bPart);
        }

        averageProb /= bundleSize;
        bundlesProb.push_back(averageProb);

        if (bundlesProb.size() > 0) {
          auto maxElement = std::max_element(bundlesProb.begin(), bundlesProb.end());
          if (*maxElement > m_PVal) {
            int maxPosition = std::distance(std::begin(bundlesProb), maxElement);
            bundles[maxPosition].addParticle(iPart);
            addedParticleToBundle = true;
          }
        }
      } //bundles
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
  } // particle Lists
}

void CurlTaggerModule::endRun()
{
}

void CurlTaggerModule::terminate()
{
}



