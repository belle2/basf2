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


#include<analysis/dataobjects/Particle.h>
#include<vector>

namespace Belle2 {
  namespace CurlTagger {

    class Selector {
      /** Abstract base class for selectors */
    public:
      Selector() {};
      ~Selector() {};

      float getProbability(Particle* iPart, Particle* jPart) {return 0;};
      std::vector<float> getVariables(Particle* iPart, Particle* jPart) {return {0, 0, 0, 0};};

    private:

    }; //class
  } // CurlTagger namespace
} // Belle 2 namespace

