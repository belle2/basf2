/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackingDQM/TrackDQMModule.h>
#include <tracking/modules/trackingDQM/TrackDQMEventProcessor.h>
#include <tracking/dqmUtils/HistogramFactory.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::HistogramFactory;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(TrackDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackDQMModule::TrackDQMModule() : DQMHistoModuleBase()
{
  setDescription("DQM of finding tracks, their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackDQMModule::initialize()
{
  DQMHistoModuleBase::initialize();

  // eventLevelTrackingInfo is currently only set by VXDTF2, if VXDTF2 is not in path the StoreObject is not there
  m_eventLevelTrackingInfo.isOptional();
}

void TrackDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2FATAL("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

  TDirectory* TracksDQM = originalDirectory->GetDirectory("TracksDQM");
  if (!TracksDQM)
    TracksDQM = originalDirectory->mkdir("TracksDQM");

  TDirectory* TracksDQMAlignment = originalDirectory->GetDirectory("TracksDQMAlignment");
  if (!TracksDQMAlignment)
    TracksDQMAlignment = originalDirectory->mkdir("TracksDQMAlignment");

  TracksDQM->cd();
  DefineTracks();
  DefineHits();
  DefineMomentumAngles();
  DefineMomentumCoordinates();
  DefineHelixParametersAndCorrelations();
  DefineTrackFitStatus();
  DefineTRClusters();
  DefineUBResidualsVXD();
  DefineHalfShellsVXD();

  DefineFlags();

  TracksDQMAlignment->cd();
  Define1DSensors();
  Define2DSensors();

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(std::get<0>(change), std::get<1>(change), std::get<2>(change));
}

void TrackDQMModule::event()
{
  DQMHistoModuleBase::event();
  if (!histogramsDefined)
    return;

  TrackDQMEventProcessor eventProcessor = TrackDQMEventProcessor(this, m_recoTracksStoreArrayName, m_tracksStoreArrayName);

  eventProcessor.Run();

  if (m_eventLevelTrackingInfo.isValid())
    m_trackingErrorFlags->Fill((double)m_eventLevelTrackingInfo->hasAnErrorFlag());
  else
    m_trackingErrorFlags->Fill(0.0);
}

void TrackDQMModule::DefineFlags()
{
  // only monitor if any flag was set so only 2 bins needed
  const char* flagTitle =
    "Tracking error summary. Mean = errors/event (should be 0 or very close to 0); Error occured yes or no; Number of events";

  m_trackingErrorFlags = Create("NumberTrackingErrorFlags", flagTitle, 2, -0.5, 1.5, "", "");

  m_trackingErrorFlags->GetXaxis()->SetBinLabel(1, "No Error");
  m_trackingErrorFlags->GetXaxis()->SetBinLabel(2, "Error occured");
}
