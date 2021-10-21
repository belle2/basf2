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
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(TrackingExpressRecoDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingExpressRecoDQMModule::TrackingExpressRecoDQMModule() : DQMHistoModuleBase()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("Data Quality Monitoring of the tracking run on ExpressReco. "
                );
  addParam("produce1Dresiduals", m_produce1Dres, "If True, produce 1D residual plots for each VXD sensor", bool(m_produce1Dres));

  addParam("produce2Dresiduals", m_produce2Dres, "If True, produce 2D residual plots for each VXD sensor", bool(m_produce2Dres));

}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingExpressRecoDQMModule::initialize()
{
  DQMHistoModuleBase::initialize();
}

void TrackingExpressRecoDQMModule::defineHisto()
{
  DQMHistoModuleBase::defineHisto();

  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

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

  if (m_produce1Dres)
    Define1DSensors();
  if (m_produce2Dres)
    Define2DSensors();

  originalDirectory->cd();

  for (auto change : m_histogramParameterChanges)
    ProcessHistogramParameterChange(get<0>(change), get<1>(change), get<2>(change));
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

}

