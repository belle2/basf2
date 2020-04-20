#include <tracking/modules/trackingDQM/TrackingEventProcessor.h>
#include <tracking/modules/trackingDQM/TrackDQMModule.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TVectorD.h>

using namespace Belle2;

TString TrackingEventProcessor::ConstructMessage()
{
  return Form("TrackDQM: track %3i, Mom: %f, %f, %f, Pt: %f, Mag: %f, Hits: PXD %i SVD %i CDC %i Suma %i\n",
              iTrack,
              (float)trackFitResult->getMomentum().Px(),
              (float)trackFitResult->getMomentum().Py(),
              (float)trackFitResult->getMomentum().Pz(),
              (float)trackFitResult->getMomentum().Pt(),
              (float)trackFitResult->getMomentum().Mag(),
              nPXD, nSVD, nCDC, nPXD + nSVD + nCDC
             );
}