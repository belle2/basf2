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
      virtual float getProbability(Particle* iPart, Particle* jPart);
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart);

    private:

    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

