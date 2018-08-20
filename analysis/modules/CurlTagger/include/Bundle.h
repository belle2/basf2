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

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/TrackVariables.h>

//#include <iostream>
#include <vector>
#include <string>
//#include <algorithm>

namespace Belle2 {
  namespace CurlTagger {

    class Bundle {
    public:
      Bundle(bool isTruthBundle);
      ~Bundle();

      void addParticle(Particle* particle);
      Particle* getParticle(unsigned int i);

      unsigned int size();

      void tagCurlInfo();
      void tagSizeInfo();

    private:
      float trackDist(Particle* particle);
      bool compareParticles(Particle* iPart, Particle* jPart);

      bool m_IsTruthBundle;
      std::string m_CurlLabel;
      std::string m_SizeLabel;
      std::vector<Particle*> m_Particles;

      double m_Gamma; // from BN1079

    };//class

  } // Curl Tagger namespace
} // Belle 2 namespace

