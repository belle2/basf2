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

#include <analysis/modules/CurlTagger/Selector.h>

#include <analysis/dataobjects/Particle.h>
#include <vector>

//Root Includes
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"


namespace Belle2 {
  namespace CurlTagger {

    class SelectorCut : public Selector {
    public:
      SelectorCut();
      ~SelectorCut();
      float getProbability(Particle* iPart, Particle* jPart);
      std::vector<float> getVariables(Particle* iPart, Particle* jPart);

    private:

    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

