/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDMaxStripTTreeModule.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace std;
using namespace SoftwareTrigger;

REG_MODULE(SVDMaxStripTTree)

SVDMaxStripTTreeModule::SVDMaxStripTTreeModule() : Module()
{
  setDescription("The module is used to create a TTree to study the number of strips per APV per event.");
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDMaxStripTTree.root"));
  addParam("ShaperDigits", m_shapersStoreArrayName, "StoreArray name of the input ShaperDigits.",
           m_shapersStoreArrayName);
  addParam("skipHLTRejectedEvents", m_skipRejectedEvents, "If TRUE skip events rejected by HLT", bool(true));
}

void SVDMaxStripTTreeModule::initialize()
{
  m_shapers.isRequired(m_shapersStoreArrayName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //Tree for SVD u and v strips
  m_t = new TTree("tree", "Tree for SVD u/v-strips");
  m_t->Branch("evt", &m_event, "evt/i");
  m_t->Branch("svdLayer", &m_svdLayer, "svdLayer/i");
  m_t->Branch("svdLadder", &m_svdLadder, "svdLadder/i");
  m_t->Branch("svdSensor", &m_svdSensor, "svdSensor/i");
  m_t->Branch("svdSide", &m_svdSide, "svdSide/i");
  m_t->Branch("svdChip", &m_svdChip, "svdChip/i");
  m_t->Branch("svdHits", &m_svdHits, "svdHits/i");

  m_event = 0;
}

void SVDMaxStripTTreeModule::beginRun()
{

  TH1F hHits("nHits_L@layerL@ladderS@sensor@view@apv",
             "Number of Hits per Event in @layer.@ladder.@sensor chip @apv on the @view/@side side",
             2, -0.5 , 1.5);

  m_hHits = new SVDAPVHistograms<TH1F>(hHits);

}


void SVDMaxStripTTreeModule::event()
{

  if (m_skipRejectedEvents && (m_resultStoreObjectPointer.isValid())) {
    const bool eventAccepted = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_resultStoreObjectPointer);
    if (!eventAccepted) return;
  }

  //count the number of strips per APV in the event
  for (const auto& shaper : m_shapers)
    m_hHits->fill(shaper.getSensorID(), shaper.isUStrip(), shaper.getCellID() / 128, 0);

  //loop on geometry and fill the tree for this event
  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
        for (int view = SVDAPVHistograms<TH1F>::VIndex ; view < SVDAPVHistograms<TH1F>::UIndex + 1; view++)
          for (int apv = 0; apv < 6; apv ++) {
            m_svdLayer = sensor.getLayerNumber();
            m_svdLadder = sensor.getLadderNumber();
            m_svdSensor = sensor.getSensorNumber();
            m_svdSide = view;
            m_svdChip = apv;
            m_svdHits = (m_hHits->getHistogram(sensor, view, apv))->GetEntries();
            m_t->Fill();

            //reset the histogram used as counters
            (m_hHits->getHistogram(sensor, view, apv))->Reset();

          }

  m_event++;
}


void SVDMaxStripTTreeModule::terminate()
{

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();
    m_t->Write();
    m_rootFilePtr->Close();
  }
}
