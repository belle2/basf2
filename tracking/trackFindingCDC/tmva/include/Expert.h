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
#include <functional>
#include <TMVA/Reader.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class Expert {
    public:
      Expert(const std::string& weightFolder, const std::string& name);
      void initializeReader(const std::function<void(TMVA::Reader&)> setReaderAddresses);

      double useWeight();
    private:
      TMVA::Reader m_reader;
      std::string m_weightFileName;
    };

  }
}
