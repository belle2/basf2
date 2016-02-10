#include <topcaf/modules/SampleTimeCalibrationV2Module/SampleTimeCalibrationV2Module.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/TopConfigurations.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/conditions/ConditionsService.h>

#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>

using namespace Belle2;

REG_MODULE(SampleTimeCalibrationV2)

void fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag);
std::vector< std::pair<double, double> > g_times;


SampleTimeCalibrationV2Module::SampleTimeCalibrationV2Module() : Module()
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

SampleTimeCalibrationV2Module::~SampleTimeCalibrationV2Module() {}

void SampleTimeCalibrationV2Module::initialize()
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

void SampleTimeCalibrationV2Module::beginRun()
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
    B2WARNING("Defaulting time/TDC to " << m_time2tdc / 1000. << " ps in SampleTimeCalibrationV2 Module.");
  }

  if (m_mode == 1) { // read mode
    TList* list = nullptr;

    if (m_conditions == 1) { // read using conditions service

      std::string filename = (ConditionsService::getInstance()->getPayloadFileURL(this));
      m_in_file = TFile::Open(filename.c_str(), "READ");

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

void SampleTimeCalibrationV2Module::event()
{
  //Get TOPdigits from datastore
  StoreArray<TOPCAFDigit> digit_ptr;
  digit_ptr.isRequired();

  if (digit_ptr) {
    for (int c = 0; c < digit_ptr.getEntries(); c++) {

      if ((m_mode == 0) && (digit_ptr[c]->GetFlag() == 1)) { // Calculate calibration
        FillWaveform(digit_ptr[c]);
      }
      if (m_mode == 1) { // Apply calibration to waveform

        double corrTime = CalibrateWaveform(digit_ptr[c]);
        double time = ((double)digit_ptr[c]->GetTime() + corrTime);
        B2INFO("s2s .. TDC_i: " << digit_ptr[c]->GetTime() << "\tcorrTime: " << corrTime << "\tTDC_c: " << time);
        digit_ptr[c]->SetTime(time);
        digit_ptr[c]->SetCorrTime(corrTime);
      }

    }

  }

}

void  SampleTimeCalibrationV2Module::terminate()
{



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

    ///////////////////////
    // ------ ODD ------ //
    ///////////////////////
    B2INFO("Number of odd events: " << v_oddevents.size());
    g_times = v_oddevents;
    const int npar = 64;
    m_Minuit = new TMinuit(npar);
    for (int i = 0; i < npar; i++) {
      m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0. , 0.01, -15., 15.);
    }
    m_Minuit->SetPrintLevel(-1);
    m_Minuit->SetFCN(fcnSamplingTime);
    m_Minuit->Migrad();
    double outpar[npar], err[npar];
    for (int i = 0; i < npar; i++) {
      m_Minuit->GetParameter(i, outpar[i], err[i]);
      //      B2INFO("par["<<i<<"]: " << outpar[i]);
    }

    //Check
    unsigned int nevents = g_times.size();
    //get average of 1th samples
    double ave_1sample = 0, n1sample(0);
    for (unsigned int i = 0; i < nevents; i++) {
      if (g_times.at(i).first == 0) {
        ave_1sample += g_times.at(i).second;
        n1sample += 1.0;
      }
    }
    ave_1sample /= (double)n1sample;
    for (unsigned int i = 0; i < nevents; i++) {
      double res = outpar[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
      m_corr_residual_h->Fill(res);
    }

    ////////////////////////
    // ------ EVEN ------ //
    ////////////////////////
    B2INFO("Number of even events: " << v_evenevents.size());
    g_times = v_evenevents;
    for (int i = 0; i < npar; i++) {
      m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0., 0.01, -15., 15.);
    }
    m_Minuit->SetPrintLevel(-1);
    m_Minuit->SetFCN(fcnSamplingTime);
    m_Minuit->Migrad();
    //double outpar[npar], err[npar];
    for (int i = 0; i < npar; i++) {
      m_Minuit->GetParameter(i, outpar[i], err[i]);
      //      B2INFO("par["<<i<<"]: " << outpar[i]);
    }

    //Check
    nevents = g_times.size();
    //get average of 1th samples
    ave_1sample = 0; n1sample = 0;
    for (unsigned int i = 0; i < nevents; i++) {
      if (g_times.at(i).first == 0) {
        ave_1sample += g_times.at(i).second;
        n1sample += 1.0;
      }
    }
    ave_1sample /= (double)n1sample;
    for (unsigned int i = 0; i < nevents; i++) {
      double res = outpar[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
      m_corr_residual_h->Fill(res);
    }

    if (m_out_file)
      m_corr_residual_h->Write();

    //Save Channel sample time info.
    std::map<topcaf_channel_id_t, TProfile*>::iterator it_ct =  m_out_sample2time.begin();
    for (; it_ct != m_out_sample2time.end(); ++it_ct) {

      topcaf_channel_id_t key =  it_ct->first;

      if (m_out_file)
        m_out_sample2time[key]->Write();

    }
    if (m_out_file)
      m_residual_h->Write();

    //TMinuit All channels
    TH1D* rms_h = new TH1D("rms_h", "rms_h", 250, 0.05, 0.15);
    std::map<topcaf_channel_id_t, v_cell_dt_pair>::iterator iter;
    for (iter = m_ch2cell_dt.begin(); iter != m_ch2cell_dt.end(); iter++) {
      topcaf_channel_id_t ch_id = iter->first;
      std::string s_channel_calib = std::to_string(ch_id);
      TH1D* channel_time_calib_h = new TH1D(s_channel_calib.c_str(), s_channel_calib.c_str(), 64, 0, 64);
      g_times = iter->second;
      B2INFO(iter->first << "\tNumber of events: " << g_times.size());
      const int npar = 64;
      m_Minuit = new TMinuit(npar);
      for (int i = 0; i < npar; i++) {
        m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0. , 0.01, -15., 15.);
      }
      m_Minuit->SetPrintLevel(-1);
      m_Minuit->SetFCN(fcnSamplingTime);
      m_Minuit->Migrad();
      double outpar[npar], err[npar];
      for (int i = 0; i < npar; i++) {
        m_Minuit->GetParameter(i, outpar[i], err[i]);
        channel_time_calib_h->Fill(i, outpar[i]);
      }
      if (m_out_file)
        channel_time_calib_h->Write();


      //Check
      nevents = g_times.size();
      //get average of 1th samples
      ave_1sample = 0; n1sample = 0;
      for (unsigned int i = 0; i < nevents; i++) {
        if (g_times.at(i).first == 0) {
          ave_1sample += g_times.at(i).second;
          n1sample += 1.0;
        }
      }
      ave_1sample /= (double)n1sample;
      TH1D* channel_time_h =  m_out_sample2time_corr[ch_id];
      if (!channel_time_h) {
        std::string s_channel_name = "TimeCalibrationCorr_" + std::to_string(ch_id);
        channel_time_h = new TH1D(s_channel_name.c_str(), s_channel_name.c_str(), 100, -15, 15);
        m_out_sample2time_corr[ch_id] = channel_time_h;
      }
      for (unsigned int i = 0; i < nevents; i++) {
        double res = outpar[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
        channel_time_h->Fill(res);
      }
      double rms = channel_time_h->GetRMS();
      if (rms < 0.5 || rms > 15.) {
        B2WARNING("Bad channel #" << ch_id << " -> RMS=" << rms);
      }
      rms_h->Fill(rms);
      channel_time_h->Write();
    }
    if (m_out_file)
      rms_h->Write();

    if (m_conditions == 1 && m_out_file)
      ConditionsService::getInstance()->writePayloadFile(m_out_file->GetName(), this);

  }






  if (m_out_file) {
    m_out_file->Close();
    delete m_out_file;
  }

  if (m_in_file) {
    m_in_file->Close();
    delete m_in_file;
  }

}


void fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag)
{
  double chi2 = npar + iflag + gin[0];
  chi2 = 0;
  unsigned int nevents = g_times.size();
  //get average of 1th samples
  double ave_1sample = 0, n1sample(0);
  for (unsigned int i = 0; i < nevents; i++) {
    if (g_times.at(i).first == 0) {
      ave_1sample += g_times.at(i).second;
      n1sample += 1.0;
    }
  }
  ave_1sample /= (double)n1sample;
  for (unsigned int i = 0; i < nevents; i++) {
    //    double chi = par[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
    double chi = par[(int)g_times.at(i).first] + g_times.at(i).second - 200.;
    chi2 += chi * chi;
  }
  f = chi2;
}

//Method used to record time valies from waveform
void SampleTimeCalibrationV2Module::FillWaveform(const TOPCAFDigit* in_digit)
{

  topcaf_channel_id_t channel_id = in_digit->GetChannelID();
  channel_id  = (channel_id / 1000);
  channel_id  = (channel_id * 1000);
  unsigned short win = in_digit->GetASICWindow();
  unsigned int refwin = in_digit->GetRefWindow();
  int coarse_int = refwin > win ? refwin - win : 64 - (win - refwin);
  coarse_int = 0;

  double rate = in_digit->GetSamplingRate();
  double max_bin = in_digit->GetTimeBin();
  double sample_t = in_digit->GetTime();
  double coarse_t = double(coarse_int) * 64.0 / rate;
  double delta_t = m_tdc + sample_t - coarse_t;
  B2DEBUG(1, "delta_t: " << delta_t << " = " << m_tdc << " + " << sample_t << " - " << coarse_t);

  int time_win = max_bin / 64.; //Which window in ROI
  int peak_win = win + time_win;
  double shift_time_bin = max_bin - 64.0 * time_win;
  B2DEBUG(1, "channel_id: " << channel_id << "\tshift_time_bin: " << shift_time_bin << " = " << max_bin << " - 64.0 * " << time_win);
  std::pair<double, double> p = std::make_pair(shift_time_bin, delta_t);


  //Start Add
  if (peak_win % 2 == 0) {
    // dt=0.356426;
    channel_id += 98;
  } else {
    // dt=0.368067;
    channel_id += 99;
  }

  if (delta_t > m_cal_mint && delta_t < m_cal_maxt) {
    TProfile* channel_time_h =  m_out_sample2time[channel_id];
    if (!channel_time_h) {
      std::string s_channel_name = "TimeCalibration_" + std::to_string(channel_id);
      channel_time_h = new TProfile(std::to_string(channel_id).c_str(), s_channel_name.c_str(), 64, 0, 64);
      m_out_sample2time[channel_id] = channel_time_h;
    }
    channel_time_h->Fill(shift_time_bin, delta_t);

    m_residual_h->Fill(delta_t);

    if (peak_win % 2 == 0) {
      v_evenevents.push_back(p);
    } else {
      v_oddevents.push_back(p);
    }

    v_cell_dt_pair this_info = m_ch2cell_dt[channel_id];
    this_info.push_back(p);
    m_ch2cell_dt[channel_id] = this_info;
    B2DEBUG(1, "channel_id: " << channel_id << "\tsamples: " << this_info.size());
  }
}


//Method used to apply calibration to waveform
double SampleTimeCalibrationV2Module::CalibrateWaveform(const TOPCAFDigit* in_digit)
{

  topcaf_channel_id_t channel_id = in_digit->GetChannelID();
  unsigned short win = in_digit->GetASICWindow();
  double time_bin = in_digit->GetTimeBin();
  int time_win = time_bin / 64.; //Which window in ROI
  int peak_win = win + time_win;
  double shift_time_bin = time_bin - 64.0 * time_win;

  topcaf_channel_id_t this_mod_chid = ((channel_id / 1000) * 1000);
  if (peak_win % 2 == 0) {
    this_mod_chid += 98;
  } else {
    this_mod_chid += 99;
  }

  // TH1D *channel_time_h =  m_in_chid2samplecalib[this_mod_chid];
  if (not m_in_chid2samplecalib[this_mod_chid]) {
    B2WARNING("Problem retrieving sample time calibration data for channel " << channel_id << "\tmod id: " << this_mod_chid);
    return -99999.;
  } else {
    double time_calib = m_in_chid2samplecalib[this_mod_chid]->GetBinContent(shift_time_bin);
    return (time_calib);
  }

}

