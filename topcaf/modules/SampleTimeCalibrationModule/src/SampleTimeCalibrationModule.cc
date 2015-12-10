#include <topcaf/modules/SampleTimeCalibrationModule/SampleTimeCalibrationModule.h>
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

REG_MODULE(SampleTimeCalibration)

void fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag);
std::vector< std::pair<double, double> > g_times;


SampleTimeCalibrationModule::SampleTimeCalibrationModule() : Module()
{
  setDescription("This module is used to create itop sample 2 sample calibration file, see the parameters for various options.  Apply these calibrations before merging the wave packets with the WaveMergingModule.  Note that this module is untested after porting from the stand alone topcaf code.");

  addParam("mode", m_mode, "Calculate Sample2Sample - 0 ; Apply Sample2Sample - 1");

  addParam("conditions", m_conditions,
           "Do not use Conditions Service - 0 ; Use Conditions Service - 1 (write to Conditions if mode==0) ", 0);
  addParam("iovInitialRunID", m_initial_run, "Initial run ID for the interval of validity for this calibration",
           std::string("NULL"));
  addParam("iovFinalRunID", m_final_run, "Final run ID for the interval of validity for this calibration", std::string("NULL"));

  addParam("inputFileName", m_in_filename, "Input filename used if mode==1 and conditions==0", std::string());
  addParam("writeFile", m_writefile, "Do not write file - 0 ; Write to local root file - 1 ", 0);
  addParam("outputFileName", m_out_filename,
           "Output filename written if mode==0 and writeFile==1, also used for temporary conditions output.",
           std::string("/tmp/temp_makesampletime.root"));

  addParam("numberOfSamples", m_channel_samples, "Number of samples to consider (4096 for IRS3C)", 4096);


  m_out_file = nullptr;
  m_in_file = nullptr;

  m_time_calib_tdc_h = nullptr;
}

SampleTimeCalibrationModule::~SampleTimeCalibrationModule() {}

void SampleTimeCalibrationModule::initialize()
{
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {
    m_out_file = TFile::Open(m_out_filename.c_str(), "recreate");
    if (!m_out_file) {
      B2FATAL("Could not write output pedestal file.  Aborting.");
    } else {
      m_out_file->Close();
    }
  }

  m_dt_h = new TH1D("m_dt_h", "m_dt_h", m_channel_samples, 0, -1);
  m_waveform_h = new TH1D("m_waveform_h", "m_waveform_h", m_channel_samples, 0, m_channel_samples);

  if (m_mode == 0) { // calculate mode
    m_time_calib_tdc_h = new TH1D("time_calib_tdc_h", "time_calib_tdc_h", m_channel_samples, 0, -1);
    m_residual_h = new TH1D("m_residual_h", "m_residual_h", m_channel_samples, 0, 0);
    m_corr_residual_h = new TH1D("m_corr_residual_h", "m_corr_residual_h", m_channel_samples, 0, 0);
  }

}

void SampleTimeCalibrationModule::beginRun()
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
    B2WARNING("Defaulting time/TDC to " << m_time2tdc / 1000. << " ps in SampleTimeCalibration Module.");
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
      if (!cl->InheritsFrom("TH1D")) continue;
      TH1D* h = (TH1D*)key->ReadObj();
      std::string name_key = h->GetName();
      topcaf_channel_id_t channel_id = strtoul(name_key.c_str(), nullptr, 0);

      m_in_chid2samplecalib[channel_id] = h;
    }

  }

}

void SampleTimeCalibrationModule::event()
{
  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();

  // Get event header from datastore
  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  m_tdc = evtheader_ptr->GetFTSW();

  //Get TOPdigits from datastore
  StoreArray<TOPDigit> topdigit_ptr;
  topdigit_ptr.isRequired();

  TOPDigits corr_topdigits;

  if (evtwaves_ptr) {
    for (int c = 0; c < evtwaves_ptr.getEntries(); c++) {

      EventWaveformPacket* evtwave_ptr = evtwaves_ptr[c];

      if ((m_mode == 0) && (evtwave_ptr->GetAmplitude() > 70)) { // Calculate calibration
        FillWaveform(evtwave_ptr);
      }
      if (m_mode == 1) { // Apply calibration to waveform

        if (topdigit_ptr) {
          TOPDigit* old_topdigit = topdigit_ptr[c];

          double corrTime = CalibrateWaveform(evtwave_ptr);
          int TDC = ((int)((double)old_topdigit->getTDC() - corrTime * m_time2tdc));
          B2INFO("s2s .. TDC_i: " << old_topdigit->getTDC() << "\tTDC_c: " << TDC << "\t corrTime: " << corrTime);
          TOPDigit* this_topdigit = new TOPDigit(old_topdigit->getModuleID(), old_topdigit->getPixelID(), TDC);
          this_topdigit->setADC(old_topdigit->getADC());

          corr_topdigits.push_back(this_topdigit);
        }

      }

    }
    if (m_mode == 1) {
      topdigit_ptr.clear();
      for (TOPDigits::iterator digit_it = corr_topdigits.begin() ; digit_it != corr_topdigits.end() ; ++digit_it) {
        topdigit_ptr.appendNew(TOPDigit(**digit_it));
      }

    }

  }
}

void  SampleTimeCalibrationModule::terminate()
{



  //If output requested then save calibration
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {
    //Save Channel sample adc info.

    if (m_conditions == 1) { // Use Conditions Service to save calibration

      m_payload_tag =  m_experiment + '_' + m_run + '_';
      m_payload_tag += getName();
      m_payload_tag += '_';
      m_payload_tag += GetVersion();


      B2INFO("writing itop Sample2Sample calibration using Conditions Service - Payload Tag:" << GetPayloadTag()
             << "\tSubsystem Tag: " << getType() << "\tAlgorithm Name: " << getName()
             << "\tVersion: " << GetVersion() << "\tRun_i: " << GetInitialRun() << "\tRun_f: " << GetFinalRun());
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
      m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0.0001 , 0.0001, -1. / 2.7135 * 2, 1. / 2.7135 * 2);
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
      m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0.0001 , 0.0001, -1. / 2.7135 / 3, 1. / 2.7135 / 3);
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
      std::string s_channel_calib = "SampleTimeCalibration_" + std::to_string(ch_id);
      TH1D* channel_time_calib_h = new TH1D(s_channel_calib.c_str(), s_channel_calib.c_str(), 64, 0, 64);
      g_times = iter->second;
      const int npar = 64;
      m_Minuit = new TMinuit(npar);
      for (int i = 0; i < npar; i++) {
        m_Minuit->DefineParameter(i, std::to_string(i).c_str(), 0.0001 , 0.0001, -2. / 2.7135, 2. / 2.7135);
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
        channel_time_h = new TH1D(std::to_string(ch_id).c_str(), s_channel_name.c_str(), 100, -1, 1);
        m_out_sample2time_corr[ch_id] = channel_time_h;
      }
      for (unsigned int i = 0; i < nevents; i++) {
        double res = outpar[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
        channel_time_h->Fill(res);
      }
      double rms = channel_time_h->GetRMS();
      if (rms < 0.5 || rms > 1.5) {
        B2WARNING("Bad channel #" << ch_id << " -> RMS=" << rms);
      }
      rms_h->Fill(rms);
      //channel_time_h->Write();
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
    double chi = par[(int)g_times.at(i).first] + g_times.at(i).second - ave_1sample;
    chi2 += chi * chi;
  }
  f = chi2;
}

//Method used to record time valies from waveform
void SampleTimeCalibrationModule::FillWaveform(const EventWaveformPacket* in_wp)
{
  topcaf_channel_id_t channel_id = in_wp->GetChannelID();
  unsigned short win = in_wp->GetASICWindow();
  unsigned int refwin = in_wp->GetRefWindow();
  int coarse_int = refwin > win ? refwin - win : 64 - (win - refwin);


  double rate = in_wp->GetSamplingRate();
  double max_bin = in_wp->GetTimeBin();
  double sample_t = in_wp->GetTime();
  double coarse_t = double(coarse_int) * 64.0 / rate;
  double delta_t = m_tdc + sample_t - coarse_t;
  //  B2INFO("delta_t: " << delta_t << " = " << m_tdc << " + " << sample_t << " - " << coarse_t);

  int time_win = max_bin / 64.; //Which window in ROI
  int peak_win = win + time_win;
  double shift_time_bin = max_bin - 64.0 * time_win;
  //  B2INFO("channel_id: " << channel_id << "\tshift_time_bin: " << shift_time_bin << " = " << max_bin << " - 64.0 * " << time_win);
  std::pair<double, double> p = std::make_pair(shift_time_bin, delta_t);

  B2INFO("channel_id: " << channel_id << "\tshift_time_bin: " << shift_time_bin << " = " << max_bin << " - 64.0 * " << time_win);

  //Start Add
  if (peak_win % 2 == 0) {
    // dt=0.356426;
    channel_id += 98;
  } else {
    // dt=0.368067;
    channel_id += 99;
  }

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
}


//Method used to apply calibration to waveform
double SampleTimeCalibrationModule::CalibrateWaveform(const EventWaveformPacket* in_wp)
{

  topcaf_channel_id_t channel_id = in_wp->GetChannelID();
  unsigned short win = in_wp->GetASICWindow();
  double time_bin = in_wp->GetTimeBin();
  int time_win = time_bin / 64.; //Which window in ROI
  int peak_win = win + time_win;
  double shift_time_bin = time_bin - 64.0 * time_win;

  //From laser or beam
  unsigned int this_mod_chid1 = floor(channel_id / 10000);
  unsigned int this_mod_chid2 = this_mod_chid1 * 100;
  unsigned int this_mod_chid3 = this_mod_chid2 + 1;
  unsigned int this_mod_chid  = this_mod_chid3 * 100;
  //Start Add
  if (peak_win % 2 == 0) {
    this_mod_chid += 98;
  } else {
    this_mod_chid += 99;
  }


  // TH1D *channel_time_h =  m_in_chid2samplecalib[this_mod_chid];
  if (not m_in_chid2samplecalib[this_mod_chid]) {
    B2WARNING("Problem retrieving sample time calibration data for channel " << channel_id);
    return -99999.;
  } else {
    double time_calib = m_in_chid2samplecalib[this_mod_chid]->GetBinContent(shift_time_bin);
    return (time_calib);
  }

}
