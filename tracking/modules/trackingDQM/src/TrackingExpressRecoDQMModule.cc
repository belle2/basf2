/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackingDQM/TrackingExpressRecoDQMModule.h>
#include <tracking/modules/trackingDQM/TrackDQMEventProcessor.h>
#include <tracking/dqmUtils/HistogramFactory.h>

#include <TDirectory.h>

using namespace Belle2;
using namespace Belle2::HistogramFactory;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(TrackingExpressRecoDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingExpressRecoDQMModule::TrackingExpressRecoDQMModule() : DQMHistoModuleBase()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Data Quality Monitoring of the tracking run on ExpressReco.");
  addParam("produce1Dresiduals", m_produce1Dres, "If True, produce 1D residual plots for each VXD sensor", m_produce1Dres);
  addParam("produce2Dresiduals", m_produce2Dres, "If True, produce 2D residual plots for each VXD sensor", m_produce2Dres);
  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory for the histograms", m_histogramDirectoryName);
  addParam("histogramTitleSuffix", m_histogramTitleSuffix,
           "Optional suffix to be appended to the title of the histograms."
           " Applied only to those histograms whose content depend on the track StoreArray chosen.",
           m_histogramTitleSuffix);
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingExpressRecoDQMModule::initialize()
{
  DQMHistoModuleBase::initialize();
  m_eventLevelTrackingInfo.isOptional();
}

void TrackingExpressRecoDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2FATAL("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

  TDirectory* TracksDQM = originalDirectory->GetDirectory(m_histogramDirectoryName.c_str());
  if (!TracksDQM)
    TracksDQM = originalDirectory->mkdir(m_histogramDirectoryName.c_str());

  TracksDQM->cd();
  DefineTracks();
  DefineHits();
  DefineHelixParametersAndCorrelations();
  DefineTrackFitStatus();
  DefineTRClusters();
  DefineUBResidualsVXD();
  DefineHalfShellsVXD();

  DefineAbortFlagsHistograms();
  if (m_produce1Dres)
    Define1DSensors();
  if (m_produce2Dres)
    Define2DSensors();

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(std::get<0>(change), std::get<1>(change), std::get<2>(change));

  // Add the title suffix (if a suffix was provided via the module parameter)
  for (TH1* hist : m_histograms) {
    // Skip histograms whose content does *not* depend on the list (StoreArray)
    // of tracks used as input to this module
    if (hist->GetName() == std::string("NumberTrackingErrorFlags"))
      continue;
    if (hist->GetName() == std::string("TrackingErrorFlagsReasons"))
      continue;
    std::string newTitle = hist->GetTitle() + m_histogramTitleSuffix;
    hist->SetTitle(newTitle.c_str());
  }
}

void TrackingExpressRecoDQMModule::event()
{
  DQMHistoModuleBase::event();
  if (!histogramsDefined)
    return;

  bool runningOnHLT = false;

  TrackDQMEventProcessor eventProcessor = TrackDQMEventProcessor(this, m_recoTracksStoreArrayName, m_tracksStoreArrayName,
                                          runningOnHLT);

  if (m_produce2Dres)
    eventProcessor.produce2Dres();
  if (m_produce1Dres)
    eventProcessor.produce1Dres();

  eventProcessor.Run();

  if (m_eventLevelTrackingInfo.isValid()) {
    if (m_eventLevelTrackingInfo->hasAnErrorFlag()) {
      m_trackingErrorFlags->Fill(1);
      if (m_eventLevelTrackingInfo->hasUnspecifiedTrackFindingFailure())
        m_trackingErrorFlagsReasons->Fill(1);
      if (m_eventLevelTrackingInfo->hasVXDTF2AbortionFlag())
        m_trackingErrorFlagsReasons->Fill(2);
      if (m_eventLevelTrackingInfo->hasSVDCKFAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(3);
      if (m_eventLevelTrackingInfo->hasPXDCKFAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(4);
      if (m_eventLevelTrackingInfo->hasSVDSpacePointCreatorAbortionFlag())
        m_trackingErrorFlagsReasons->Fill(5);
    } else {
      m_trackingErrorFlags->Fill(0);
      m_trackingErrorFlagsReasons->Fill(0);
    }
  } else
    m_trackingErrorFlags->Fill(0);
}

void TrackingExpressRecoDQMModule::DefineAbortFlagsHistograms()
{
  // only monitor if any flag was set so only 2 bins needed
  m_trackingErrorFlags =
    Create("NumberTrackingErrorFlags",
           "Tracking error summary. Mean = errors/event (should be 0 or very close to 0);Error occured yes or no;Number of events",
           2, -0.5, 1.5, "Error occured yes or no", "Number of events");
  m_trackingErrorFlags->GetXaxis()->SetBinLabel(1, "No Error");
  m_trackingErrorFlags->GetXaxis()->SetBinLabel(2, "Error occured");


  m_trackingErrorFlagsReasons =
    Create("TrackingErrorFlagsReasons",
           "Tracking errors by reason. A single event may fall in multiple bins.;Type of error occurred;Number of events",
           6, -0.5, 5.5, "Type of error occurred", "Number of events");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(1, "No Error");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(2, "Unspecified PR");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(3, "VXDTF2");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(4, "SVDCKF");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(5, "PXDCKF");
  m_trackingErrorFlagsReasons->GetXaxis()->SetBinLabel(6, "SpacePoint");

}
