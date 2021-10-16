/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackingDQM/TrackingERDQMModule.h>
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

REG_MODULE(TrackingERDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingERDQMModule::TrackingERDQMModule() : DQMHistoModuleBase()
{
  setDescription("Data Quality Monitoring of the tracking run on ExpressReco. "
                );
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingERDQMModule::initialize()
{
  DQMHistoModuleBase::initialize();
}

void TrackingERDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

  // There might be problems with nullptr if the directory with the same name already exists (but I am not sure because there isn't anything like that in AlignmentDQM)
  TDirectory* TracksDQM = originalDirectory->GetDirectory("TrackingERDQM");
  if (!TracksDQM)
    TracksDQM = originalDirectory->mkdir("TrackingERDQM");

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

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(get<0>(change), get<1>(change), get<2>(change));
}

void TrackingERDQMModule::event()
{
  DQMHistoModuleBase::event();
  if (!histogramsDefined)
    return;

  bool runningOnHLT = false;

  TrackDQMEventProcessor eventProcessor = TrackDQMEventProcessor(this, m_recoTracksStoreArrayName, m_tracksStoreArrayName,
                                          runningOnHLT);

  eventProcessor.Run();

}

