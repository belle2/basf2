/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/tmva/TMVA.h>
#include <vector>
#include <string>

class TFile;
class TTree;
class TCut;

namespace Belle2 {
  namespace TrackFindingCDC {

    class Teacher {
    public:
      Teacher();
      void createWeights(const std::string& weightFolder, TFile& outputFile, TTree& eventTree,
                         const std::vector<std::string>& variableNames, const std::vector<std::string>& spectatorNames,
                         const TCut& signalCut, const TCut& backgroundCut,
                         const std::string optionString = "NTrees=100:Shrinkage=0.1:RandRatio=0.5:NCutLevel=7");
    };

  }
}
