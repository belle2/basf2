#include <topcaf/modules/SampleTimeCalibrationV5Module/SampleTimeCalibrationV5Module.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/TopConfigurations.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
//#include <framework/conditions/ConditionsService.h>

#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>
#include "TH2D.h"

using namespace Belle2;

REG_MODULE(SampleTimeCalibrationV5)

namespace topcafSampleTimeCalibrationV5MinuitParameters {

  //TMinuit function SetFCN() requires a global fuction.
  //Put function and associated parameters into separate namespace to avoid truly global variables.
  void fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag);
  std::vector< hit_info > g_hitinfo;
  std::vector< double > g_dt_ave;
  std::map< topcaf_channel_id_t, std::vector<double> > g_averages;
  int g_cycles;
  TH2D h1o("h1o", "time1 diff vs sample", 256, 0, 256, 100, 190, 210);
  TH2D h2o("h2o", "time2 diff vs sample", 256, 0, 256, 100, 190, 210);

  TH2D h1f("h1f", "time1 diff vs sample", 256, 0, 256, 100, 190, 210);
  TH2D h2f("h2f", "time2 diff vs sample", 256, 0, 256, 100, 190, 210);
}

SampleTimeCalibrationV5Module::SampleTimeCalibrationV5Module() : Module()
{
  setDescription("This module is used to create itop sample 2 sample calibration file, see the parameters for various options.  Apply these calibrations before merging the wave packets with the WaveMergingModule.  Note that this module is untested after porting from the stand alone topcaf code.");

  addParam("mode", m_mode, "Calculate Sample2Sample - 0 ; Apply Sample2Sample - 1");

  addParam("conditions", m_conditions,
           "Do not use Conditions Service - 0 ; Use Conditions Service - 1 (write to Conditions if mode==0) ", 0);

  addParam("inputFileName", m_in_filename, "Input filename used if mode==1 and conditions==0", std::string());
  addParam("writeFile", m_writefile, "Do not write file - 0 ; Write to local root file - 1 ", 0);
  addParam("outputFileName", m_out_filename,
           "Output filename written if mode==0 and writeFile==1, also used for temporary conditions output.",
           std::string("/tmp/temp_makesampletime.root"));

  addParam("numberOfSamples", m_channel_samples, "Number of samples to consider (4096 for IRS3C)", 4096);
  addParam("minCalTime", m_cal_mint, "Only hits with time>minCalTime are considered for this calibration.", 190.);
  addParam("maxCalTime", m_cal_maxt, "Only hits with time<minCalTime are considered for this calibration.", 210.);



  m_out_file = nullptr;
  m_in_file = nullptr;

  m_time_calib_tdc_h = nullptr;
}

SampleTimeCalibrationV5Module::~SampleTimeCalibrationV5Module() {}

void SampleTimeCalibrationV5Module::initialize()
{
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {
    m_out_file = TFile::Open(m_out_filename.c_str(), "recreate");
    if (!m_out_file) {
      B2FATAL("Could not write output pedestal file.  Aborting.");
    } else {
      m_out_file->Close();
    }
  }

  //  m_dt_h = new TH1D("m_dt_h", "m_dt_h", m_channel_samples, 0, -1);
  //  m_waveform_h = new TH1D("m_waveform_h", "m_waveform_h", m_channel_samples, 0, m_channel_samples);

  if (m_mode == 0) { // calculate mode
    //    m_time_calib_tdc_h = new TH1D("time_calib_tdc_h", "time_calib_tdc_h", m_channel_samples, 0, -1);
    m_residual_h = new TH1D("m_residual_h", "m_residual_h", m_channel_samples, 0, 0);
    m_corr_residual_h = new TH1D("m_corr_residual_h", "m_corr_residual_h", m_channel_samples, 0, 0);
  }

}

void SampleTimeCalibrationV5Module::beginRun()
{

  StoreObjPtr<topFileMetaData> metadata_ptr;
  metadata_ptr.isRequired();

  if (metadata_ptr) {
    m_experiment = metadata_ptr->getExperiment();
    m_run = metadata_ptr->getRun();
  }

  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1. / (topconfig_ptr->getTDCUnit_ns());
  } else {
    m_time2tdc = 1000.;
    B2WARNING("Defaulting time/TDC to " << m_time2tdc / 1000. << " ps in SampleTimeCalibrationV5 Module.");
  }

  if (m_mode == 1) { // read mode
    TList* list = nullptr;

    if (m_conditions == 1) { // read using conditions service
      /*FIXME
            std::string filename = (ConditionsService::getInstance()->getPayloadFileURL(this));
            m_in_file = TFile::Open(filename.c_str(), "READ");
      */
    } else if (m_conditions == 0) { // read  from local file

      m_in_file = TFile::Open(m_in_filename.c_str(), "READ");

    }

    if (!m_in_file) {
      B2ERROR("Couldn't open input file: " << m_in_filename);
    }  else {
      list = m_in_file->GetListOfKeys();
    }

    /// Now load up the calibration
    TIter next(list);
    TKey* key;
    while ((key = (TKey*)next())) {
      TClass* cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TH1D") || cl->InheritsFrom("TProfile")) continue;
      TH1D* h = (TH1D*)key->ReadObj();
      std::string name_key = h->GetName();
      topcaf_channel_id_t channel_id = strtoul(name_key.c_str(), nullptr, 0);
      channel_id = ((channel_id / 10000) * 10000) + (channel_id % 100);
      B2DEBUG(1, "channel_id: " << channel_id << "\tobject name: " << name_key << "\tobject class: " << key->GetClassName());
      m_in_chid2samplecalib[channel_id] = h;
    }

  }

}

void SampleTimeCalibrationV5Module::event()
{
  //Get TOPdigits from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  if (digit_ptr) {
    if (m_mode == 0) { // Calculate calibration
      std::vector<int> cal10_pulses;
      std::vector<int> cal11_pulses;
      for (int c = 0; c < digit_ptr.getEntries(); c++) { // First id the cal pulse for each asic.
        if (digit_ptr[c]->GetFlag() == 10) {
          cal10_pulses.push_back(c);
        }
        if (digit_ptr[c]->GetFlag() == 11) {
          cal11_pulses.push_back(c);
        }
      }

      //Take no chances: match up the cal pulses to the same asic
      for (unsigned int d = 0; d < cal10_pulses.size(); d++) {

        topcaf_channel_id_t cal10_channel_id = digit_ptr[cal10_pulses[d]]->GetChannelID();
        topcaf_channel_id_t cal10_asic_id = ((cal10_channel_id / 1000) * 1000);
        int cal10_win = digit_ptr[cal10_pulses[d]]->GetASICWindow();
        int cal10_win_corr = (cal10_win % 4) * 64;

        for (unsigned int e = 0; e < cal11_pulses.size(); e++) {

          topcaf_channel_id_t cal11_channel_id = digit_ptr[cal11_pulses[e]]->GetChannelID();
          topcaf_channel_id_t cal11_asic_id = ((cal11_channel_id / 1000) * 1000);
          int cal11_win = digit_ptr[cal11_pulses[e]]->GetASICWindow();
          int cal11_win_corr = (cal11_win % 4) * 64;

          if (cal10_asic_id == cal11_asic_id) {
            //B2DEBUG(1, "phit\tphoton_win_corr: " << photon_win_corr << "\tcal_win_corr: " << cal_win_corr);
            hit_info this_hit_info;
            this_hit_info.sample1 = digit_ptr[cal10_pulses[d]]->GetTDCBin() + cal10_win_corr;
            this_hit_info.sample2 = digit_ptr[cal11_pulses[e]]->GetTDCBin() + cal11_win_corr;
            m_cal_photon_pairs[cal10_asic_id].push_back(this_hit_info);
            //B2DEBUG(1, "pair\tdigi1: " << digit_ptr[cal_pulses[d]]->GetTDCBin() << "\ts1: " << this_hit_info.sample1 << "\tdigi2: " << (double)digit_ptr[cal_pulses[d]]->GetTDCBin() << "\ts2: " << this_hit_info.sample2);
          }
        }
      }
    }

    for (int c = 0; c < digit_ptr.getEntries(); c++) {

      if (m_mode == 1) { // Apply calibration to waveform

        double corrTime = CalibrateWaveform(digit_ptr[c]);
        double time = ((double)digit_ptr[c]->GetTime() - corrTime);
        B2DEBUG(1, "s2s .. TDC_i: " << digit_ptr[c]->GetTime() << "\tcorrTime: " << corrTime << "\tTDC_c: " << time);
        //        digit_ptr[c]->SetTime(time);
        //        digit_ptr[c]->SetCorrTime(corrTime);

      }

    }

  }

}

void  SampleTimeCalibrationV5Module::terminate()
{
  using namespace topcafSampleTimeCalibrationV5MinuitParameters;

  //If output requested then save calibration
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {
    //Save Channel sample adc info.

    if (m_conditions == 1) { // Use Conditions Service to save calibration


      B2INFO("writing itop Sample2Sample calibration using Conditions Service");
      if (m_writefile == 0)
        m_out_file = TFile::Open(m_out_filename.c_str(), "recreate");

    }
    if (m_writefile == 1) {

      B2INFO("writing itop " << getName() << " calibration file manually to " << m_out_filename);
      m_out_file = TFile::Open(m_out_filename.c_str(), "recreate");

    }

    if (m_out_file)
      m_out_file->cd();


    if (m_time_calib_tdc_h) {
      if (m_out_file) {
        m_time_calib_tdc_h->Write();
      }
    }

    for (auto iter = m_cal_photon_pairs.begin(); iter != m_cal_photon_pairs.end(); iter++) {

      topcaf_channel_id_t ch_id = iter->first;
      //      if(ch_id==6602020000){
      {
        const int npar = 255;
        std::string s_channel_calib = std::to_string(ch_id);
        TH1D* channel_time_calib_h = new TH1D(s_channel_calib.c_str(), s_channel_calib.c_str(), npar, 0, npar);
        g_hitinfo = iter->second;


        TMinuit myMinuit(npar);
        for (int i = 0; i < npar; i++) {
          myMinuit.DefineParameter(i, std::to_string(i).c_str(), 1. , 0.01, 0.1, 3.0);
        }
        myMinuit.SetPrintLevel(-1);
        myMinuit.SetFCN(fcnSamplingTime);
        g_cycles = 0;
        myMinuit.Migrad();
        B2INFO("asic ch_id: " << ch_id << "\tnumber of hits: " << g_hitinfo.size() << "\tminuit calls: " << g_cycles;);
        double outpar[npar], err[npar];
        for (int i = 0; i < npar; i++) {
          myMinuit.GetParameter(i, outpar[i], err[i]);
          channel_time_calib_h->SetBinContent(i + 1, outpar[i]);
          channel_time_calib_h->SetBinError(i + 1, err[i]);
          B2INFO("par[" << i << "]: " << outpar[i]);
        }
        if (m_out_file) {
          channel_time_calib_h->Write();
        }

      }

    }
    if (m_out_file) {
      h1o.Write();
      h2o.Write();
      h1f.Write();
      h2f.Write();
    }


//FIXME    if (m_conditions == 1 && m_out_file)
//      ConditionsService::getInstance()->writePayloadFile(m_out_file->GetName(), this);

  }

  // Close any open files.
  if (m_out_file) {
    m_out_file->Close();
    delete m_out_file;
  }

  if (m_in_file) {
    m_in_file->Close();
    delete m_in_file;
  }

}


void topcafSampleTimeCalibrationV5MinuitParameters::fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag)
{
  using namespace topcafSampleTimeCalibrationV5MinuitParameters;

  double chisq = npar + iflag + gin[0];
  chisq = 0.;
  double ave = 0.;
  std::vector<double> time_diffs;
  int total = npar + 1;
  h1f.Reset();
  h2f.Reset();
  for (unsigned int c = 0; c < g_hitinfo.size(); c++) {
    int time1_nwin = g_hitinfo[c].sample1 / total;
    int time2_nwin = g_hitinfo[c].sample2 / total;
    double sample1 = fmod(g_hitinfo[c].sample1, total);
    double sample2 = fmod(g_hitinfo[c].sample2, total);
    double time1 = 0.;
    double time2 = 0.;
    double samp1_frac = modf(sample1, &sample1);
    double samp2_frac = modf(sample2, &sample2);

    /// First do sample 1 time calculation
    int d = 0;
    for (d = 0; d < sample1; d++) {
      time1 += par[d];
    }
    if (d < npar) {
      time1 += samp1_frac * par[d];
    } else if (d == npar) { /// Enforce that the total is 256 samples... (npar+1)
      time1 += samp1_frac * (total - time1);
    }
    time1 += time1_nwin * total;
    /// sample 1 time done.

    //// now do sample 2 time calculation
    for (d = 0; d < sample2; d++) {
      time2 += par[d];
    }
    if (d < npar) {
      time2 += samp2_frac * par[d];
    } else if (d == npar) { /// Enforce that the total is 256 samples... (npar+1)
      time2 += samp2_frac * (total - time2);
    }
    time2 += time2_nwin * total;

    double time_diff = time2 - time1; // assume time2>time1...
    ave += time_diff;
    time_diffs.push_back(time_diff);

    h1f.Fill(sample1, time_diff);
    h2f.Fill(sample2, time_diff);
    if (g_cycles == 0) {
      h1o.Fill(sample1, time_diff);
      h2o.Fill(sample2, time_diff);

      //      B2INFO("s1: "<<sample1<<"\ts2: "<<sample2<<"\ttime1: "<<time1<<"\ttime2: "<<time2<<"\tdt: "<<time_diff);
    }


  }
  ave /= g_hitinfo.size(); // Now we have the average, find the chi2...
  if (g_cycles == 0) {
  }
  for (unsigned int c = 0; c < time_diffs.size(); c++) {
    chisq += (time_diffs[c] - ave) * (time_diffs[c] - ave);
  }
  g_cycles++;

  f = chisq;

}


//Method used to apply calibration to waveform
double SampleTimeCalibrationV5Module::CalibrateWaveform(TOPCAFDigit* in_digit)
{

  topcaf_channel_id_t channel_id = in_digit->GetChannelID();
  topcaf_channel_id_t this_mod_chid = ((channel_id / 1000) * 1000);



  // TH1D *channel_time_h =  m_in_chid2samplecalib[this_mod_chid];
  if (not m_in_chid2samplecalib[this_mod_chid]) {
    B2WARNING("Problem retrieving sample time calibration data for channel " << channel_id << "\tmod id: " << this_mod_chid);
    return -99999.;
  } else {
    unsigned short win = in_digit->GetASICWindow();
    int win_corr = (win % 4) * 64;
    double sample1 = in_digit->GetTDCBin() + win_corr; // Correct the time bin.
    double sample2 = in_digit->GetTDCBin() + in_digit->GetWidth() + win_corr; // correct the width too.
    int total = m_in_chid2samplecalib[this_mod_chid]->GetNbinsX() + 1;
    int time1_nwin = sample1 / total;
    int time2_nwin = sample2 / total;
    sample1 = fmod(sample1, total);
    sample2 = fmod(sample2, total);
    double time1 = 0.;
    double time2 = 0.;
    double samp1_frac = modf(sample1, &sample1);
    double samp2_frac = modf(sample2, &sample2);

    int c = 0;
    for (c = 0; c < sample1; c++) {
      time1 += m_in_chid2samplecalib[this_mod_chid]->GetBinContent(c + 1);
    }
    if (c < (total - 1)) {
      time1 += samp1_frac * m_in_chid2samplecalib[this_mod_chid]->GetBinContent(c + 1);
    } else { /// Enforce that the total is 256 samples... (npar+1)
      time1 += samp1_frac * (total - time1);
    }
    time1 += time1_nwin * total;

    for (c = 0; c < sample2; c++) {
      time2 += m_in_chid2samplecalib[this_mod_chid]->GetBinContent(c + 1);
    }
    if (c < (total - 1)) {
      time2 += samp2_frac * m_in_chid2samplecalib[this_mod_chid]->GetBinContent(c + 1);
    } else { /// Enforce that the total is 256 samples... (npar+1)
      time2 += samp2_frac * (total - time2);
    }
    time2 += time2_nwin * total;
    double delta = (in_digit->GetTDCBin() + win_corr) - (time1);
    B2DEBUG(1, "channel_id: " << channel_id << "\ts_i: " << in_digit->GetTDCBin() << "\ts_f: " << time1 - win_corr << "\tdelta: " <<
            delta);
    in_digit->SetTDCBin(time1);
    in_digit->SetWidth((time2 - time1));


    return (delta);
  }

}
