/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <svd/persistenceManager/PersistenceManager.h>

struct Counters {
  int event;
  int run;
  int experiment;
  int production;
  int track;
  int cluster;
  unsigned int nClusters;
};

namespace Belle2::SVD {
  class SVDValidationModule : public Module {
  public:
    SVDValidationModule();

    virtual ~SVDValidationModule() = default;

    virtual void initialize() override;

    virtual void event() override;

    virtual void terminate() override;

  private:
    std::string m_fileName;
    std::string m_containerName;

    std::vector<std::string> m_variablesToNtuple;
    std::vector<std::tuple<std::string, int, float, float>> m_variablesToHistogram;

    std::string m_recoTracksStoreArrayName{"RecoTracks"};
    std::unique_ptr<PersistenceManager> persistenceManager;
  };
}
