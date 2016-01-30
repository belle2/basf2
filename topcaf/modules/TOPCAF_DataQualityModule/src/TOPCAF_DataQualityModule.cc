#include <framework/core/HistoModule.h>
#include <topcaf/modules/TOPCAF_DataQualityModule/TOPCAF_DataQualityModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <framework/pcore/RbTuple.h>
#include <utility>
#include "TDirectory.h"

using namespace Belle2;
using namespace std;

REG_MODULE(TOPCAF_DataQuality)


TOPCAF_DataQualityModule::TOPCAF_DataQualityModule()
  : HistoModule()
  , m_iEvent(0)
{
  setDescription("TOPCAF DQM histogram module");
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", string("TOPCAF"));
}


void TOPCAF_DataQualityModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();
  m_samples = new TH1F("ADCvalues", "ADC values ", 100, 0, 2000);
  m_samples->GetXaxis()->SetTitle("ADC Value");
  m_samples->GetYaxis()->SetTitle("Number of Samples");
  oldDir->cd();
}


void TOPCAF_DataQualityModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM;
}


void TOPCAF_DataQualityModule::beginRun()
{
}


void TOPCAF_DataQualityModule::event()
{
  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();
  if (not evtwaves_ptr) {
    return;
  }

  for (int c = 0; c < evtwaves_ptr.getEntries(); c++) {
    EventWaveformPacket* evtwave_ptr = evtwaves_ptr[c];
    auto channelID = evtwave_ptr->GetChannelID();
    const vector<double> v_samples = evtwave_ptr->GetSamples();
    size_t nsamples = v_samples.size();
    if (m_channelNoiseMap.find(channelID) == m_channelNoiseMap.end()) {
      string idName = string("noise_") + to_string(channelID);
      m_channelNoiseMap.insert(make_pair(channelID, new TH1F(idName.c_str(), idName.c_str(), 200, -100, 100)));
      m_channelEventMap.insert(make_pair(channelID, m_iEvent));
    }
    TH1F* noise = m_channelNoiseMap[channelID];
    // Plot all samples in common histogram for quick sanity check
    for (size_t s = 0; s < nsamples; s++) {
      double adc = v_samples.at(s);
      m_samples->Fill(adc);
      if (s < nsamples - 1) {
        noise->Fill(v_samples[s + 1] - adc);
      }
    }
  }
  m_iEvent += 1;
  return;
}


void TOPCAF_DataQualityModule::endRun()
{
  for (map<topcaf_channel_id_t, TH1F*>::iterator it = m_channelNoiseMap.begin(); it != m_channelNoiseMap.end(); ++it) {
    // if more than 1% of ADC values differ by a large number from the previous value, consider channel noisy.
    if ((*it).second->Integral(50, 150) < 0.98 * (*it).second->GetEntries()) {
      B2INFO("Channel " << (*it).first << " (e.g. in event " << m_channelEventMap[(*it).first] + 1 << ") is noisy.");
    }
  }

}


void TOPCAF_DataQualityModule::terminate()
{
}
