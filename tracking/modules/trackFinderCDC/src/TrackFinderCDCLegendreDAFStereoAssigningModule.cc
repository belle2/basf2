#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreDAFStereoAssigningModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/DAF.h>
#include <genfit/Exception.h>
#include <genfit/MeasurementFactory.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <TDatabasePDG.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>


using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreDAFStereoAssigning)

CDCLegendreDAFStereoAssigningModule::CDCLegendreDAFStereoAssigningModule()
{

}

CDCLegendreDAFStereoAssigningModule::~CDCLegendreDAFStereoAssigningModule()
{

}

void CDCLegendreDAFStereoAssigningModule::initialize()
{


}

void CDCLegendreDAFStereoAssigningModule::event()
{

  B2FATAL("This module is currently under refactoring!");
  return;

}
