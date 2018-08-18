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

      virtual float getProbability(Particle* iPart, Particle* jPart) {return -1.;};
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart) {return { -1., -1., -1.};};

    private:

    }; //class
  } // CurlTagger namespace
} // Belle 2 namespace

