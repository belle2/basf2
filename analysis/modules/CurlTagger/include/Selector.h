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
      /** Abstract base class for curl track selectors */
    public:
      /** Constructor */
      Selector() {};

      /** Destructor */
      ~Selector() {};

      /** returns prob that two particles are actual from same mc/data particle */
      virtual float getProbability(Particle* iPart, Particle* jPart) {return -1.;};

      /** returns vector of variables used by the selector */
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart) {return { -1., -1., -1.};};

    }; //class
  } // CurlTagger namespace
} // Belle 2 namespace

