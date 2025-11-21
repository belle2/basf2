/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingCDC/findlets/complete/CDCHitsRemover.h>
#include <framework/core/ModuleParamList.templateDetails.h>


using namespace Belle2;
using namespace TrackFindingCDC;

void CDCHitsRemover::initialize()
{
  Super::initialize();
  m_cdcHits.isRequired();
  m_cdc_selector.registerSubset(m_cdcHits, DataStore::EStoreFlags::c_WriteOut);
};


void CDCHitsRemover::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->getParameter<std::string>("inputWireHits").setDefaultValue("CDCWireHitVector");
}

std::string CDCHitsRemover::getDescription()
{
  return "Removes CDCHits which are marked as background";
}


void CDCHitsRemover::apply(const std::vector<CDCWireHit>& wireHits)
{
  unsigned int counter = 0;
  std::unordered_set<unsigned> indicesToRemove;
  for (const TrackFindingCDC::CDCWireHit& wireHit : wireHits) {
    counter += 1;
    if (wireHit->hasBackgroundFlag()) {
      indicesToRemove.insert(counter);
    };
  };

  // Selector
  auto selector = [indicesToRemove, this](const RelationsObject * p) -> bool {
    int idx = p->getArrayIndex();
    return (indicesToRemove.count(idx) == 0);
  };

  m_cdc_selector.select(selector);
};
