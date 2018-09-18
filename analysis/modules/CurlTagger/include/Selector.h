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

    /** Abstract base class for curl track selectors */
    class Selector {

    public:
      /** Constructor */
      Selector() {};

      /** Destructor */
      ~Selector() {};

      /** returns prob that two particles are actual from same mc/data particle */
      virtual float getResponse(Particle*, Particle*) = 0;

      /** returns vector of variables used by the selector */
      virtual std::vector<float> getVariables(Particle*, Particle*) = 0;

      /** initialise selector if needed */
      virtual void initialize() {};

      /** finalise selector if needed */
      virtual void finalize() {};

      /** collect information for training for mva or other selectors*/
      virtual void collectTrainingInfo(Particle*, Particle*)  {};
    }; //class
  } // CurlTagger namespace
} // Belle 2 namespace

