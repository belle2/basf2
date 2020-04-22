#include <tracking/modules/TrackDQM/TrackDQMEventProcessor.h>
#include <tracking/modules/TrackDQM/TrackDQMModule.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TVectorD.h>

using namespace Belle2;

TString TrackDQMEventProcessor::ConstructMessage()
{
  return Form("TrackDQM: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
              m_iTrack,
              (float)m_trackFitResult->getMomentum().Px(),
              (float)m_trackFitResult->getMomentum().Py(),
              (float)m_trackFitResult->getMomentum().Pz(),
              (float)m_trackFitResult->getMomentum().Pt(),
              (float)m_trackFitResult->getMomentum().Mag(),
              m_nPXD, m_nSVD, m_nCDC, m_nPXD + m_nSVD + m_nCDC
             );
}