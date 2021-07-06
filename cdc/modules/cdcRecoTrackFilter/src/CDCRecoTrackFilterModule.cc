/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "cdc/modules/cdcRecoTrackFilter/CDCRecoTrackFilterModule.h"
#include <tracking/dataobjects/RecoHitInformation.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(CDCRecoTrackFilter)

CDCRecoTrackFilterModule::CDCRecoTrackFilterModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  setDescription("use this module to exclude Layers in fitting, after TrackFinding");
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collection to hold Belle2::RecoTrack", std::string(""));
  addParam("ExcludeSLayer", m_excludeSLayer, "Super layers (0-8) not used in the fitting", std::vector<unsigned short> {});
  addParam("ExcludeICLayer", m_excludeICLayer, "layers (0-55) not used in the fitting", std::vector<unsigned short> {});
}

CDCRecoTrackFilterModule::~CDCRecoTrackFilterModule()
{
}

void CDCRecoTrackFilterModule::initialize()
{
  m_RecoTracks.isRequired(m_recoTrackArrayName);
}

void CDCRecoTrackFilterModule::beginRun()
{
}

void CDCRecoTrackFilterModule::event()
{
  // Loop over Recotracks
  int nTr = m_RecoTracks.getEntries();
  for (int i = 0; i < nTr; ++i) {
    const RecoTrack* track = m_RecoTracks[i];
    BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * cdchit, track->getCDCHitList()) {
      WireID wireid(cdchit->getID());
      unsigned short slay = wireid.getISuperLayer();
      unsigned short iclay = wireid.getICLayer();
      for (unsigned short j = 0; j < m_excludeSLayer.size(); ++j) {
        if (slay == m_excludeSLayer.at(j)) {
          track->getRecoHitInformation(cdchit)->setUseInFit(false);
        }
      }

      for (unsigned short j = 0; j < m_excludeICLayer.size(); ++j) {
        if (iclay == m_excludeICLayer.at(j)) {
          track->getRecoHitInformation(cdchit)->setUseInFit(false);
        }
      }

    }//end of track (Boost_foreach)
  }//end RecoTrack array
}//End Event
void CDCRecoTrackFilterModule::endRun()
{
}

void CDCRecoTrackFilterModule::terminate()
{
}

