/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackingDQM/TrackingHLTDQMModule.h>
#include <tracking/modules/trackingDQM/TrackDQMEventProcessor.h>
#include <tracking/dqmUtils/HistogramFactory.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::HistogramFactory;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(TrackingHLTDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingHLTDQMModule::TrackingHLTDQMModule() : DQMHistoModuleBase()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Data Quality Monitoring of the tracking run on HLT. "
                );
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingHLTDQMModule::initialize()
{
  DQMHistoModuleBase::initialize();

  // eventLevelTrackingInfo is currently only set by VXDTF2, if VXDTF2 is not in path the StoreObject is not there
  m_eventLevelTrackingInfo.isOptional();
}

void TrackingHLTDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();
  DQMHistoModuleBase::runningOnHLT();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

  // There might be problems with nullptr if the directory with the same name already exists (but I am not sure because there isn't anything like that in AlignmentDQM)
  TDirectory* TracksDQM = originalDirectory->GetDirectory("TrackingHLTDQM");
  if (!TracksDQM)
    TracksDQM = originalDirectory->mkdir("TrackingHLTDQM");

  TracksDQM->cd();
  DefineTracks();
  DefineHits();
  DefineMomentumAngles();
  DefineMomentumCoordinates();
  DefineHelixParametersAndCorrelations();
  DefineTrackFitStatus();

  DefineFlags();

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(get<0>(change), get<1>(change), get<2>(change));
}

void TrackingHLTDQMModule::event()
{
  DQMHistoModuleBase::event();
  if (!histogramsDefined)
    return;

  bool runningOnHLT = true;

  TrackDQMEventProcessor eventProcessor = TrackDQMEventProcessor(this, m_recoTracksStoreArrayName, m_tracksStoreArrayName,
                                          runningOnHLT);

  eventProcessor.Run();

  if (m_eventLevelTrackingInfo.isValid())
    m_trackingErrorFlags->Fill((double)m_eventLevelTrackingInfo->hasAnErrorFlag());
  else
    m_trackingErrorFlags->Fill(0.0);
}

void TrackingHLTDQMModule::DefineFlags()
{
  // only monitor if any flag was set so only 2 bins needed
  const char* flagTitle =
    "Tracking error summary. Mean = errors/event (should be 0 or very close to 0); Error occured yes or no; Number of events";

  m_trackingErrorFlags = Create("NumberTrackingErrorFlags", flagTitle, 2, -0.5, 1.5, "", "");

  m_trackingErrorFlags->GetXaxis()->SetBinLabel(1, "No Error");
  m_trackingErrorFlags->GetXaxis()->SetBinLabel(2, "Error occured");
}
