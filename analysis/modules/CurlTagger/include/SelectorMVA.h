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
#include <string>

#include <mva/methods/FastBDT.h>
#include <mva/interface/Interface.h>

//Root Includes
#include "TFile.h"
#include "TTree.h"

namespace Belle2 {
  namespace CurlTagger {

    /** Simple cut based selector for curl tracks taken from Belle note 1079 */
    class SelectorMVA : public Selector {

    public:
      /** Constructor */
      SelectorMVA(bool belleFlag, bool trainFlag);

      /** Destructor */
      ~SelectorMVA();

      /** Probability that this pair of particles come from the same mc/actual particle */
      virtual float getProbability(Particle* iPart, Particle* jPart);

      /** returns vector of variables used by this selector. Not really useful here but included to be consistent with eventual MVA method. */
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart);

      /** initialize whatever needs to be initalized (root file etc)  */
      virtual void initialize();

      /** output training event data to ttree*/
      virtual void collect(Particle* iPart, Particle* jPart);

      /** finalize whatever needs to be finalized (train the MVA) */
      virtual void finalize();

    private:

      /** updates the value of the MVA variable */
      void updateVariables(Particle* iPart, Particle* jPart);

      /** applying mva or training it */
      bool m_TrainFlag;

      //
      MVA::GeneralOptions m_generalOptions;
      MVA::FastBDTExpert m_expert;

      // Objects
      std::string m_TFileName;
      TFile* m_TFile;
      TTree* m_TTree;


      // General Options data - just use same names
      std::string m_identifier;
      std::vector<std::string> m_datafiles;
      std::vector<std::string> m_variables;
      std::string m_target_variable;

      // MVA Variables
      /** */
      Float_t m_PtDiff;
      Float_t m_PzDiff;
      Float_t m_PPhi;
      Float_t m_D0Diff;
      Float_t m_Z0Diff;
      Float_t m_PvalDiff;
      Float_t m_TrackTanLambdaDiff;
      Float_t m_ChargeMult;

      //truth
      Float_t m_IsCurl;
    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

