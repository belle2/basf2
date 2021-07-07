/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
      virtual ~Selector() {};

      /** returns selector response that two particles are actual from same mc/data particle */
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

