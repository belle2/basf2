#include <topcaf/modules/WaveTimingModule/WaveTimingModule.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <framework/datastore/StoreArray.h>
#include "TH1D.h"
#include "TSpline.h"
#include <cmath>

using namespace Belle2;

REG_MODULE(WaveTiming);

const int channel_samples = 4096;

WaveTimingModule::WaveTimingModule() : Module()
{
  setDescription("This module calculates the timing of itop raw waveform event data");
}

WaveTimingModule::~WaveTimingModule() {}

void WaveTimingModule::initialize()
{

  m_topdigits_ptr.registerInDataStore();
  m_fraction = 0.4;
  m_time_delay = 15;
  m_rate = 1.0;
  m_crude_time = 0;
  m_cf_time = 0;
  m_tmp_h = new TH1D("wave_h", "wave_h", channel_samples, 0, channel_samples);
}

void WaveTimingModule::event()
{

  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();

  //Output TOPDIgit
  m_topdigits_ptr.clear();

  if (evtwaves_ptr) {

    //Look over all waveforms
    for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {

      unsigned int channel_id = evtwaves_ptr[w]->GetChannelID();
      double win_time_shift = evtwaves_ptr[w]->GetTime();
      std::vector< double > v_samples = evtwaves_ptr[w]->GetSamples();
      int refwin = evtwaves_ptr[w]->GetRefWindow();
      int win = evtwaves_ptr[w]->GetASICWindow();
      int coarse_int = refwin > win ? refwin - win : 64 - (win - refwin);
      float window_dt = (1. / 0.0212) / 2.0;
      float sample_dt = window_dt / 64.0;
      float coarse_t = float(coarse_int) * window_dt;

      //Find rough peak location
      m_tmp_h->Reset();
      for (unsigned int s = 0; s < v_samples.size(); s++) {
        m_tmp_h->Fill(s, v_samples.at(s));
      }

      //Remove any noise artifacts by smoothing
      m_tmp_h->Smooth();

      //Refine max time
      double max_bin = (double) m_tmp_h->GetMaximumBin();
      TSpline5* s5 = new TSpline5(m_tmp_h);
      double min = max_bin - 5;
      double max = max_bin + 5;
      double steps = 10000.0;
      double step_size = (max - min) / steps;
      double max_adc = 0;
      double max_t = 0;
      for (int i = 0; i < steps; i++) {
        double this_t = min + i * step_size;
        double val = s5->Eval(this_t);
        if (val > max_adc) {
          max_adc = val;
          max_t = this_t;
        }
      }
      //Find 40%
      double damp_best = 9999;
      double at40_adc = 0;
      double at40_t = 0;
      double min40 = 0;
      double max40 = max_bin;
      double steps40 = 10000.0;
      double step_size40 = (max40 - min40) / steps40;
      for (int i = 0; i < steps40; i++) {
        double this_t = min40 + i * step_size40;
        double val = s5->Eval(this_t);
        double damp = fabs(val - 0.4 * max_adc);
        if (damp_best > damp) {
          damp_best = damp;
          at40_t = this_t;
        }
      }
      delete s5;
      max_t = at40_t;

      //Create TOPDigit
      int barID = 0;
      int channelID = channel_id;
      int TDC = at40_t * sample_dt;
      TOPDigit* this_topdigit = m_topdigits_ptr.appendNew(barID, channelID, TDC);
      this_topdigit->setADC(max_adc);

    }//End loop
  }

}

void WaveTimingModule::terminate()
{

}
