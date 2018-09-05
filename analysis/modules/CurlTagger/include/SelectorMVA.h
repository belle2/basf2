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

    /** MVA based selector for tagging curl tracks in Belle and Belle II */
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

      /** collect training data and save to a root file*/
      virtual void collectTrainingInfo(Particle* iPart, Particle* jPart);

      /** finalize whatever needs to be finalized (train the MVA) */
      virtual void finalize();

    private:

      /** updates the value of the MVA variable */
      void updateVariables(Particle* iPart, Particle* jPart);

      /** applying mva or training it */
      bool m_TrainFlag;

      /** name of output file for training data */
      std::string m_TFileName;

      /**output file for training data */
      TFile* m_TFile;

      /**training data tree */
      TTree* m_TTree;

      /** mva general options (for the expert)*/
      MVA::GeneralOptions m_generalOptions;

      /** mva expert */
      MVA::FastBDTExpert m_expert;


      // General Options data - just use same names
      /** mva identifier */
      std::string m_identifier;

      /** name of datafile */
      std::vector<std::string> m_datafiles;

      /** names of variables used by mva */
      std::vector<std::string> m_variables;

      /** name of target variable (isCurl) */
      std::string m_target_variable;


      // MVA Variables
      /** */
      Float_t m_PtDiff;
      Float_t m_PzDiff;
      Float_t m_PPhi;
      Float_t m_TrackD0Diff;
      Float_t m_TrackZ0Diff;
      Float_t m_TrackPValueDiff;
      Float_t m_TrackTanLambdaDiff;
      Float_t m_ChargeMult;
      Float_t m_TrackPhi0Diff;
      Float_t m_TrackOmegaDiff;

      Float_t m_PtDiffEW;
      Float_t m_PzDiffEW;
      Float_t m_TrackD0DiffEW;
      Float_t m_TrackZ0DiffEW;
      Float_t m_TrackTanLambdaDiffEW;
      Float_t m_TrackPhi0DiffEW;
      Float_t m_TrackOmegaDiffEW;

      /** isCurl Truth */
      Float_t m_IsCurl;
    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

