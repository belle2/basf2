#include <topcaf/modules/SampleTimeCalibrationV6Module/SampleTimeCalibrationV6Module.h>
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

REG_MODULE(SampleTimeCalibrationV6)

namespace topcafSampleTimeCalibrationV6MinuitParameters {

  //TMinuit function SetFCN() requires a global fuction.
  //Put function and associated parameters into separate namespace to avoid truly global variables.
  void fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag);
  std::vector< hit_info > g_hitinfo;
  std::vector< double > g_dt_ave;
  std::map< topcaf_channel_id_t, std::vector<double> > g_averages;
  int g_cycles;
  TH2D* g_h1o = nullptr;
  TH2D* g_h2o = nullptr;
  TH2D* g_h1f = nullptr;
  TH2D* g_h2f = nullptr;
}

SampleTimeCalibrationV6Module::SampleTimeCalibrationV6Module() : Module()
{
  setDescription("This module is used to create itop sample 2 sample calibration file, see the parameters for various options.  Apply these calibrations before merging the wave packets with the WaveMergingModule.  This V6 module implements an alternative to the minuit based fitter.");

  addParam("mode", m_mode, "Calculate Sample2Sample - 0 ; Apply Sample2Sample - 1");

  addParam("conditions", m_conditions,
           "Do not use Conditions Service - 0 ; Use Conditions Service - 1 (write to Conditions if mode==0) ", 0);

  addParam("inputFileName", m_in_filename, "Input filename used if mode==1 and conditions==0", std::string());
  addParam("writeFile", m_writefile, "Do not write file - 0 ; Write to local root file - 1 ", 0);
  addParam("outputFileName", m_out_filename,
           "Output filename written if mode==0 and writeFile==1, also used for temporary conditions output.",
           std::string("/tmp/temp_makesampletime.root"));

  addParam("numberOfSamples", m_channel_samples, "Number of samples to consider (4096 for IRS3C)", 4096);
  addParam("minCalTime", m_cal_mint, "Only hits with time>minCalTime are considered for this calibration.", 190.0);
  addParam("maxCalTime", m_cal_maxt, "Only hits with time<minCalTime are considered for this calibration.", 210.0);

  addParam("minTimeDiff", m_min_time_diff, "Minimum time difference to be used in dT minimisation.", 0.0);
  addParam("maxTimeDiff", m_max_time_diff, "Maximum time difference to be used in dT minimisation.", 256.0);
  addParam("useMinuit", m_useMinuit, "Use minuit based fitter: 1; Use non-minuit based fitter: 0.", 1);
  addParam("invertHitOrder", m_invertHitOrder,
           "Set to true to invert the order of hits, for example when the first hit is expected to be later than the second hit in the input files.",
           false);
  addParam("smoothSamples", m_smoothSamples,
           "Smooth fit results by replacing results in unphysical bins (or ranges of bins) by interpolating between neighbours.", false);
  addParam("minimumSampleWidth", m_minimumSampleWidth,
           "With smoothSamples the minimum width of the sample bin, in terms of multiples of the default bin width, before smoothing is applied.",
           0.0);
  addParam("maximumSampleWidth", m_maximumSampleWidth,
           "With smoothSamples the maximum width of the sample bin, in terms of multiples of the default bin width, before smoothing is applied.",
           2.0);
  addParam("nIterations", m_nIterations, "Number of iterations to use for the minimiser.", 20);
  addParam("nMinimiserBins", m_nMinimiserBins, "Number of bins to use in the width histograms used by the minimiser.", 400);


  m_out_file = nullptr;
  m_in_file = nullptr;

  m_time_calib_tdc_h = nullptr;
}

SampleTimeCalibrationV6Module::~SampleTimeCalibrationV6Module()
{
}

void SampleTimeCalibrationV6Module::initialize()
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

void SampleTimeCalibrationV6Module::beginRun()
{
  StoreObjPtr<topFileMetaData> metadata_ptr;
  metadata_ptr.isRequired();

  if (metadata_ptr) {
    m_experiment = metadata_ptr->getExperiment();
    m_run = metadata_ptr->getRun();
  }

  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  if (topconfig_ptr) {
    m_time2tdc = 1.0 / (topconfig_ptr->getTDCUnit_ns());
    B2INFO("Using time/TDC of " << m_time2tdc / 1000.0 << " ps in SampleTimeCalibrationV6 Module.");
  } else {
    m_time2tdc = 1000.;
    B2WARNING("Defaulting time/TDC to " << m_time2tdc / 1000.0 << " ps in SampleTimeCalibrationV6 Module.");
  }
  // m_time2tdc is never used after this.
  B2INFO("Note: This parameter (time/TDC) is not actually used currently.");

  if (m_invertHitOrder) {
    B2INFO("Order of hits will be inverted during module processing.");
  } else {
    B2DEBUG(101, "Order of hits will not be inverted during module processing.");
  }


  if (m_mode == 1) { // read mode
    TList* list = nullptr;

    if (m_conditions == 1) { // read using conditions service
      /* FIXME
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

void SampleTimeCalibrationV6Module::event()
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
            if (m_invertHitOrder) {
              //invert order of hits - can be useful in cases where the first hit is expected to be later than the second hit
              double swap_variable = this_hit_info.sample2;
              this_hit_info.sample2 = this_hit_info.sample1;
              this_hit_info.sample1 = swap_variable;
            }

            //make some monitoring histograms:
            if (m_sample_occupancies_hit1.find(cal10_asic_id) == m_sample_occupancies_hit1.end()) {
              TString occupancy_histo_name("occ_hit1_");
              occupancy_histo_name += cal10_asic_id;
              m_sample_occupancies_hit1[cal10_asic_id] = new TH1D(occupancy_histo_name, "occupancy - hit1", 256, 0.0, 256.0);
              m_sample_occupancies_hit1[cal10_asic_id]->SetXTitle("sample number % 256");
              m_sample_occupancies_hit1[cal10_asic_id]->SetYTitle("number of hits");
            }
            m_sample_occupancies_hit1[cal10_asic_id]->Fill(((int(this_hit_info.sample1)) % 256));

            if (m_sample_occupancies_hit2.find(cal11_asic_id) == m_sample_occupancies_hit2.end()) {
              TString occupancy_histo_name("occ_hit2_");
              occupancy_histo_name += cal11_asic_id;
              m_sample_occupancies_hit2[cal11_asic_id] = new TH1D(occupancy_histo_name, "occupancy - hit2", 256, 0.0, 256.0);
              m_sample_occupancies_hit2[cal11_asic_id]->SetXTitle("sample number % 256");
              m_sample_occupancies_hit2[cal11_asic_id]->SetYTitle("number of hits");
            }
            m_sample_occupancies_hit2[cal11_asic_id]->Fill(((int(this_hit_info.sample2)) % 256));

            //double time_difference = this_hit_info.sample2 - this_hit_info.sample1;
            //Float_t winDt(m_time2tdc * 128);  // ns per window
            //Float_t winDt2(2.0 * winDt); // ns per window

            //double time_difference(0);
            //if ((time2 - time1) > 0) {time_difference = (this_hit_info.sample2 - this_hit_info.sample1);}
            //if ((time2 - time1) < 0) {time_difference = ((winDt2 + this_hit_info.sample2) - this_hit_info.sample1);}




            if (m_sample_occupancies_vs_tdiff_hit1.find(cal10_asic_id) == m_sample_occupancies_vs_tdiff_hit1.end()) {
              TString occupancy_histo_name("smp_vs_tdiff_hit1_");
              occupancy_histo_name += cal10_asic_id;
              m_sample_occupancies_vs_tdiff_hit1[cal10_asic_id] = new TH2D(occupancy_histo_name, "time difference vs sample - hit1", 256, 0.0,
                  256.0, m_nMinimiserBins, m_min_time_diff, m_max_time_diff);
              m_sample_occupancies_vs_tdiff_hit1[cal10_asic_id]->SetXTitle("sample number % 256");
              m_sample_occupancies_vs_tdiff_hit1[cal10_asic_id]->SetYTitle("time difference / default bin widths");
            }
            //m_sample_occupancies_vs_tdiff_hit1[cal10_asic_id]->Fill(62.0, 62.0);

            if (m_sample_occupancies_vs_tdiff_hit2.find(cal11_asic_id) == m_sample_occupancies_vs_tdiff_hit2.end()) {
              TString occupancy_histo_name("smp_vs_tdiff_hit2_");
              occupancy_histo_name += cal11_asic_id;
              m_sample_occupancies_vs_tdiff_hit2[cal11_asic_id] = new TH2D(occupancy_histo_name, "time difference vs sample - hit2", 256, 0.0,
                  256.0, m_nMinimiserBins, m_min_time_diff, m_max_time_diff);
              m_sample_occupancies_vs_tdiff_hit2[cal11_asic_id]->SetXTitle("sample number % 256");
              m_sample_occupancies_vs_tdiff_hit2[cal11_asic_id]->SetYTitle("time difference / default bin widths");
            }
            //m_sample_occupancies_vs_tdiff_hit2[cal11_asic_id]->Fill(((int(this_hit_info.sample2)) % 256), time_difference);

            if (m_samples_hit1_vs_hit2.find(cal11_asic_id) == m_samples_hit1_vs_hit2.end()) {
              TString histo_name("smp1_vs_smp2_");
              histo_name += cal11_asic_id;
              TString title = "Sample1 vs Sample2 for channel ";
              title += cal11_asic_id;
              m_samples_hit1_vs_hit2[cal11_asic_id] = new TH2D(histo_name, title, 256, 0.0, 256.0, 256, 0.0, 256.0);
              m_samples_hit1_vs_hit2[cal11_asic_id]->SetXTitle("Sample1 % 256");
              m_samples_hit1_vs_hit2[cal11_asic_id]->SetYTitle("Sample2 % 256");
            }
            m_samples_hit1_vs_hit2[cal11_asic_id]->Fill(((int(this_hit_info.sample1)) % 256), ((int(this_hit_info.sample2)) % 256));



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

void  SampleTimeCalibrationV6Module::terminate()
{
  using namespace topcafSampleTimeCalibrationV6MinuitParameters;

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
      m_out_file->mkdir("extra", "extra");

    }

    if (m_out_file) {m_out_file->cd();}


    if (m_time_calib_tdc_h) {
      if (m_out_file) {
        m_time_calib_tdc_h->Write();
      }
    }

    // output a summary of number of hits:
    B2DEBUG(1, "Number of hits per channel/asic available for calibration:");
    for (auto iter = m_cal_photon_pairs.begin(); iter != m_cal_photon_pairs.end(); iter++) {
      topcaf_channel_id_t ch_id = iter->first;
      g_hitinfo = iter->second;
      B2DEBUG(50, "Channel_id: " << ch_id << "\thits available: " << iter->second.size());
    }

    //Write monitoring histograms into output file directory "extra":
    if (m_out_file) {
      m_out_file->cd("extra");
      for (auto it1(m_sample_occupancies_hit1.begin()); it1 != m_sample_occupancies_hit1.end(); ++it1) {
        it1->second->Write();
      }
      for (auto it2(m_sample_occupancies_hit2.begin()); it2 != m_sample_occupancies_hit2.end(); ++it2) {
        it2->second->Write();
      }
//      for (auto it(m_sample_occupancies_vs_tdiff_hit1.begin()); it != m_sample_occupancies_vs_tdiff_hit1.end(); ++it) {
//        it->second->Write();
//      }
//      for (auto it(m_sample_occupancies_vs_tdiff_hit2.begin()); it != m_sample_occupancies_vs_tdiff_hit2.end(); ++it) {
//        it->second->Write();
//      }
      for (auto it(m_samples_hit1_vs_hit2.begin()); it != m_samples_hit1_vs_hit2.end(); ++it) {
        it->second->Write();
      }
      m_out_file->cd();
    }
    for (auto iter = m_cal_photon_pairs.begin(); iter != m_cal_photon_pairs.end(); iter++) {
      topcaf_channel_id_t ch_id = iter->first;
      g_hitinfo = iter->second;
      B2INFO("Calibrating channel " << ch_id);

      if (m_useMinuit) {
        B2INFO("Using Minuit based fitting procedure for sample time calibration.");

        const int npar = 255;
        std::string s_channel_calib = std::to_string(ch_id);
        m_channel_time_calib_h = new TH1D(s_channel_calib.c_str(), s_channel_calib.c_str(), npar, 0, npar);
        m_channel_time_calib_h->SetXTitle("sample number % 256");
        m_channel_time_calib_h->SetYTitle("dT / default bin widths");

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
          m_channel_time_calib_h->SetBinContent(i + 1, outpar[i]);
          m_channel_time_calib_h->SetBinError(i + 1, err[i]);
          B2INFO("par[" << i << "]: " << outpar[i]);
        }

      } else { //use non-minuit based fitter
        //if (2100000000 != ch_id) {continue;}
        B2INFO("Using non-Minuit based fitting procedure for sample time calibration.");
        B2INFO("Calibrating channel " << ch_id << " using " << g_hitinfo.size() << " hits.");

        Float_t winDt(m_time2tdc * 128);  // ns per window
        Float_t winDt2(2.0 * winDt); // ns per window
        Float_t nomDt = winDt2 / 255.0;

        //overwrite these values until m_time2tdc has meaningful value, use default bin widths instead:
        nomDt  = 1.0;
        winDt2 = nomDt * 255.0;
        winDt  = winDt2 / 2.0;

        //Float_t m_dTval[257];
        Float_t tWidth(-1);


        // initialize starting dT values
        for (Int_t i(0); i < 256; ++i) {m_dTval[i] = i * nomDt;} // start with the nominal dT values
        //dTval[256] = winDt2;  // wrap constraint
        m_dTval[256] = winDt2;

        //Float_t lowVal(0);
        //Float_t highVal(260);
        TString name("h_dTmin");
        TString title("Calibration");
        title += ch_id;
        TH1F* h1 = new TH1F(name, title, m_nMinimiserBins, m_min_time_diff, m_max_time_diff);
        h1->SetXTitle("Sample number % 256");
        h1->SetYTitle("time difference / default sample widths");
        //m_iteration_vs_tdiff = new TH2D()
        //name  = "smp_vs_smp_" + ch_id;
        //title = "Sample1 vs Sample2 for channel" + ch_id;
        //m_samples_hit1_vs_hit2 = new TH2D(name, title, 256, 0.0, 256.0, 256, 0.0, 256.0);
        name = "iteration_vs_tdiff_";
        name += ch_id;
        title = "Width vs iteration for channel ";
        title += ch_id;
        m_iteration_vs_tdiff = new TH2D(name, title, m_nIterations, 0.0, m_nIterations, m_nMinimiserBins, m_min_time_diff, m_max_time_diff);
        m_iteration_vs_tdiff->SetXTitle("Iteration number");
        m_iteration_vs_tdiff->SetYTitle("width / default sample widths");
        //Fill h1 for the first time:
        for (auto it(g_hitinfo.begin()); it != g_hitinfo.end(); ++it) {
          int total = 256;
          int time1_nwin = it->sample1 / total;
          int time2_nwin = it->sample2 / total;
          double sample1 = fmod(it->sample1, total);
          double sample2 = fmod(it->sample2, total);
          double time1 = 0.;
          double time2 = 0.;
          double sample1_frac = modf(sample1, &sample1);
          double sample2_frac = modf(sample2, &sample2);

          int s1index(sample1);
          int s2index(sample2);
          s1index = s1index % 256;
          s2index = s2index % 256;

          time1 = m_dTval[s1index];
          time1 += sample1_frac * (m_dTval[s1index + 1] - m_dTval[s1index]);

          time2 = m_dTval[s2index];
          time2 += sample2_frac * (m_dTval[s2index + 1] - m_dTval[s2index]);

          double time_diff(0);
          if ((time2 - time1) > 0) {time_diff = (time2 - time1);}
          if ((time2 - time1) < 0) {time_diff = ((winDt2 + time2) - time1);}

          time1  += time1_nwin * total;
          time2  += time2_nwin * total;

          //double time_diff;
          //if (time2 > time1) {time_diff = time2 - time1;}
          //else {time_diff = time1 - time2;}
          //if((time2 - time1) > 0) {time_diff = (time2 - time1);}
          //if((time2 - time1) < 0) {time_diff = ((winDt2 + time2) - time1);}

          h1->Fill(time_diff);
          m_iteration_vs_tdiff->Fill(0.0 , time_diff);
          m_sample_occupancies_vs_tdiff_hit1[ch_id]->Fill(s1index, time_diff);
          m_sample_occupancies_vs_tdiff_hit2[ch_id]->Fill(s2index, time_diff);

          //Fill other histograms:
          //m_samples_hit1_vs_hit2->Fill(sample1, sample2);

          //B2INFO("time1 = " << time1 << "\ttime2 = " << time2 << "\ttime_diff = " << time_diff);
          //B2INFO("h1->GetMean() = " << h1->GetMean() << "\th1->GetRMS() = " << h1->GetRMS());
        }

        B2INFO("h1->GetMean() = " << h1->GetMean() << "\th1->GetRMS() = " << h1->GetRMS());
        TString h1_initial_title("h1_");
        h1_initial_title += ch_id;
        h1_initial_title += "_initial";
        TH1F* h1_initial = (TH1F*) h1->Clone(h1_initial_title);

        m_out_file->cd("extra");
        h1_initial->Write();
        m_out_file->cd();

        Float_t tryRMS, aMean, tryDt[256];
        Float_t bestRMS = 10.0;
        Int_t   moBettah;
        Float_t stepDt, stepDtRef = 0.1;  // try adaptive step size ;

        for (Int_t nSize = 0; nSize < m_nIterations; nSize++) {
          stepDtRef /= 1.2;  // ad hoc 20% reduction each pass (perhaps not make magic number)
          for (Int_t nOuter = 1; nOuter < 256; nOuter++) {
            moBettah = 1;
            while (moBettah) {
              stepDt = stepDtRef;
              moBettah = 0; // abort loop unless get at least one improvement

              // sign loop
              for (Int_t nSign = 0; nSign < 2; nSign++) {
                for (Int_t nI = 0; nI < 256; nI++) {tryDt[nI] = m_dTval[nI];} // assign starting place
                // update timing  for trial exchanges
                tryDt[nOuter] = m_dTval[nOuter] + stepDt * (0.5 - 1.0 * nSign);

                h1->Reset();  // clear for check

                // loop over events

                //for (Int_t ev=0; ev<iEvt-1; ev++) {
                for (auto it(g_hitinfo.begin()); it != g_hitinfo.end(); ++it) {
                  // re-calculate
                  int total = 256;

                  int time1_nwin = it->sample1 / total;
                  int time2_nwin = it->sample2 / total;
                  double sample1 = fmod(it->sample1, total);
                  double sample2 = fmod(it->sample2, total);
                  double time1 = 0.;
                  double time2 = 0.;
                  double sample1_frac = modf(sample1, &sample1);
                  double sample2_frac = modf(sample2, &sample2);

                  int s1index(sample1);
                  int s2index(sample2);
                  s1index = s1index % 256;
                  s2index = s2index % 256;

                  //B2INFO("it.sample1 = " << it->sample1 << "\tsample1index = " << s1index << "\tsample1_frac = " << sample1_frac);
                  //B2INFO("it.sample2 = " << it->sample2 << "\tsample2index = " << s2index << "\tsample2_frac = " << sample2_frac);
                  double tLeading = 0;
                  double tTrailing = 0;

                  //tLeading = tryDt[s1index] + sample1_frac * (tryDt[s1index+1] - tryDt[s1index]);
                  //tTrailing = tryDt[s2index] + sample2_frac * (tryDt[s2index+1] - tryDt[s2index]);

                  // tLeading  -> time1
                  // tTrailing -> time2

                  time1 = tryDt[s1index];
                  time1 += sample1_frac * (tryDt[s1index + 1] - tryDt[s1index]);

                  time2 = tryDt[s2index];
                  time2 += sample2_frac * (tryDt[s2index + 1] - tryDt[s2index]);

                  double time_diff(0);
                  if ((time2 - time1) > 0) {time_diff = (time2 - time1);}
                  if ((time2 - time1) < 0) {time_diff = ((winDt2 + time2) - time1);}

                  time1  += time1_nwin * total;
                  time2  += time2_nwin * total;

                  //double time_diff = time2 - time1;
                  //double time_diff;
                  //if (time2 > time1) {time_diff = time2 - time1;}
                  //else {time_diff = time1 - time2;}
                  //if((time2 - time1) > 0) {time_diff = (time2 - time1);}
                  //if((time2 - time1) < 0) {time_diff = ((winDt2 + time2) - time1);}
                  tLeading  += time1_nwin * total;
                  tTrailing += time2_nwin * total;

                  //if((tTrailing-tLeading)>0) {tWidth = (tTrailing-tLeading);}
                  //if((tTrailing-tLeading)<0) {tWidth = ((winDt2+tTrailing)-tLeading);}
                  //B2INFO("time1 = " << time1 << "\ttime2 = " << time2 << "\ttime_diff = " << time_diff);
                  //B2INFO("tLeading = " << tLeading << "\ttTrailing = " << tTrailing << "\ttWidth = " << tWidth);
                  tWidth = time_diff;

                  h1->Fill(time_diff);
                }  // evts loop

                aMean  = h1->GetMean();
                tryRMS = h1->GetRMS();

                if (tryRMS < bestRMS) {

                  B2DEBUG(2, "bestRMS = " << tryRMS << " for nOuter = " << nOuter);
                  B2DEBUG(2, "tryDT[nOuter] = " << tryDt[nOuter] << " for stepDT = " << (-1.0 + 2 * nSign)*stepDt);
                  B2DEBUG(2, "tWidth = " << tWidth << " and Mean = " << aMean);
                  //B2INFO("bestRMS = " << tryRMS << " for nOuter = " << nOuter);
                  //B2INFO("tryDT[nOuter] = " << tryDt[nOuter] << " for stepDT = " << (-1.0 + 2 * nSign)*stepDt);
                  //B2INFO("tWidth = " << tWidth << " and Mean = " << aMean);

                  m_dTval[nOuter] = tryDt[nOuter];
                  moBettah = 1; // try loop again
                  bestRMS = tryRMS;
                }

              }  // nSign loop

            }  // moBettah loop

          }  // nOuter (sample position) loop
          B2INFO("Finished iteration " << nSize + 1 << ": h1->GetRMS() = " << h1->GetRMS());
          for (int iBin(1); iBin < m_nMinimiserBins; ++iBin) {
            m_iteration_vs_tdiff->SetBinContent(nSize + 1, iBin, h1->GetBinContent(iBin));
          }


        }  // nSize loop

        //Fill output histogram
        std::string output_name = std::to_string(ch_id);

        TString output_title("Calibration for channel ");
        output_title += ch_id;
        output_title += " calculated using ";
        output_title += g_hitinfo.size();
        output_title += " hits";
        m_channel_time_calib_h = new TH1D(output_name.c_str(), output_title, 255, 0, 255);
        output_name += "_cumulative";
        m_channel_time_calib_cumulative_h = new TH1D(output_name.c_str(), output_title, 255, 0, 255);
        for (int i = 0; i < 256; ++i) {
          m_channel_time_calib_h->SetBinContent(i + 1, m_dTval[i + 1] - m_dTval[i]);
          m_channel_time_calib_cumulative_h->SetBinContent(i + 1, m_dTval[i]);
          //channel_time_calib_h->SetBinError(i + 1, err[i]);
          B2DEBUG(1, "par[" << i << "]: " << m_dTval[i]);
        }

        TString h1_final_title("h1_");
        h1_final_title += ch_id;
        h1_final_title += "_final";
        TH1F* h1_final = (TH1F*) h1->Clone(h1_final_title);
        m_out_file->cd("extra");
        h1_final->Write();
        m_out_file->cd();
        delete h1;
      } // non-minuit based fitter
      // Write out sample time calibration histograms (same format for both minuit and non-minuit based fitter.
      if (m_out_file) {
        if (m_smoothSamples) {
          TString cloneName(m_channel_time_calib_h->GetName());
          cloneName += "_original";
          TH1D* original_channel_time_calib_h = (TH1D*) m_channel_time_calib_h->Clone(cloneName);
          m_out_file->cd("extra");
          original_channel_time_calib_h->Write();
          m_out_file->cd();
          smoothSampleHistogram(m_channel_time_calib_h);
        }
        m_channel_time_calib_h->Write();

        m_out_file->cd("extra");
        m_channel_time_calib_cumulative_h->Write();
        m_iteration_vs_tdiff->Write();
        m_out_file->cd();
      }
      delete m_channel_time_calib_h;
      makeClosurePlots(ch_id, g_hitinfo);
    } // end loop over cal_photon_pairs

    //make closure plots
//    makeClosurePlots(ch_id, g_hitinfo);

    if (m_out_file) {
      if (g_h1o) {
        g_h1o->Write();
      }
      if (g_h2o) {
        g_h2o->Write();
      }
      if (g_h1f) {
        g_h1f->Write();
      }
      if (g_h2f) {
        g_h2f->Write();
      }
      m_out_file->cd("extra");
      for (auto it(m_sample_occupancies_vs_tdiff_hit1.begin()); it != m_sample_occupancies_vs_tdiff_hit1.end(); ++it) {
        it->second->Write();
      }
      for (auto it(m_sample_occupancies_vs_tdiff_hit2.begin()); it != m_sample_occupancies_vs_tdiff_hit2.end(); ++it) {
        it->second->Write();
      }
      m_out_file->cd();
    }


    if (m_conditions == 1 && m_out_file) {
//FIXME      ConditionsService::getInstance()->writePayloadFile(m_out_file->GetName(), this);
    }

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


void topcafSampleTimeCalibrationV6MinuitParameters::fcnSamplingTime(int& npar, double* gin, double& f, double* par, int iflag)
{
  using namespace topcafSampleTimeCalibrationV6MinuitParameters;

  double chisq = npar + iflag + gin[0];
  chisq = 0.;
  double ave = 0.;
  std::vector<double> time_diffs;
  int total = npar + 1;
  if (not g_h1o) {
    g_h1o = new TH2D("h1o", "time1 diff vs sample", 256, 0, 256, 100, 190, 210);
  }
  if (not g_h2o) {
    g_h2o = new TH2D("h2o", "time2 diff vs sample", 256, 0, 256, 100, 190, 210);
  }
  if (not g_h1f) {
    g_h1f = new TH2D("h1f", "time1 diff vs sample", 256, 0, 256, 100, 190, 210);
  }
  if (not g_h2f) {
    g_h2f = new TH2D("h2f", "time2 diff vs sample", 256, 0, 256, 100, 190, 210);
  }

  g_h1f->Reset();
  g_h2f->Reset();
  for (unsigned int c = 0; c < g_hitinfo.size(); c++) {
    int time1_nwin = g_hitinfo[c].sample1 / total;
    int time2_nwin = g_hitinfo[c].sample2 / total;
    double sample1 = fmod(g_hitinfo[c].sample1, total);
    double sample2 = fmod(g_hitinfo[c].sample2, total);
    double time1 = 0.;
    double time2 = 0.;
    double samp1_frac = modf(sample1, &sample1);
    double samp2_frac = modf(sample2, &sample2);


    B2INFO("sample1 = " << g_hitinfo[c].sample1 << "\ttime1_nwin = " << time1_nwin << "\tsample1 = " << sample1 << "\tsample1_frac = "
           << samp1_frac);
    B2INFO("sample2 = " << g_hitinfo[c].sample2 << "\ttime2_nwin = " << time2_nwin << "\tsample2 = " << sample2 << "\tsample2_frac = "
           << samp2_frac);

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

    B2INFO("time1 = " << time1 << "\ttime2 = " << time2 << "\ttime_diff = " << time_diff);


    g_h1f->Fill(sample1, time_diff);
    g_h2f->Fill(sample2, time_diff);
    if (g_cycles == 0) {
      g_h1o->Fill(sample1, time_diff);
      g_h2o->Fill(sample2, time_diff);

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
double SampleTimeCalibrationV6Module::CalibrateWaveform(TOPCAFDigit* in_digit)
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


void SampleTimeCalibrationV6Module::smoothSampleHistogram(TH1D* histogram)
{

  B2DEBUG(57, "Smoothing sample histogram " << histogram->GetName());

  for (int i(1); i < 257; ++i) {
    B2DEBUG(57, "Checking bin " << i << " with width " << histogram->GetBinContent(i));

    if (histogram->GetBinContent(i) < m_minimumSampleWidth || histogram->GetBinContent(i) > m_maximumSampleWidth) {
      B2DEBUG(57, "Need to modify sample " << i << "and neighbours.");
      // Find next bin which passes sample width checks, and the sum of all bins between this bin and that bin passes
      // scaled sample width checks.  Then average accross selected bins - this method could be improved upon.

      int j(1);
      double sum(histogram->GetBinContent(i));
      for (; j < (257 - i); ++j) {
        sum += histogram->GetBinContent(i + j);
        bool sumCheck = (sum > (j + 1) * m_minimumSampleWidth && sum < (j + 1) * m_maximumSampleWidth);
        bool neighbourCheck = (histogram->GetBinContent(i + j) > m_minimumSampleWidth
                               && histogram->GetBinContent(i + j) < m_maximumSampleWidth);
        if (sumCheck && neighbourCheck) {break;}
      }

      for (int k(0); k <= j; ++k) {
        B2DEBUG(57, "Setting bin " << i + k << " width to " <<  sum / (j + 1));
        histogram->SetBinContent(i + k, sum / (j + 1));
      }
      i += j;
    }
  }
}



void SampleTimeCalibrationV6Module::getSampleNumbersAndFractions(Belle2::hit_info* this_hit_info, int& sample1, int& sample2,
    double& fraction1, double& fraction2)
{

  double sample1temp = fmod(this_hit_info->sample1, 256);
  double sample2temp = fmod(this_hit_info->sample2, 256);

  fraction1 = modf(sample1temp, &sample1temp);
  fraction2 = modf(sample2temp, &sample2temp);

  sample1 = int(sample1temp);
  sample2 = int(sample2temp);
  sample2 = sample2 % 256;
  sample2 = sample2 % 256;

}

void SampleTimeCalibrationV6Module::makeClosurePlots(topcaf_channel_id_t ch_id, std::vector<hit_info>& hitInfoVector)
{

  B2INFO("In SampleTimeCalibrationV6Module::makeClosurePlots.");


  TString closureHistogramName("closure_sample1_");
  closureHistogramName += ch_id;
  TString closureHistogramTitle("Closure histogram for sample 1 for channel ");
  closureHistogramTitle += ch_id;
  m_closure_tdiff_vs_sample1 = new TH2D(closureHistogramName, closureHistogramTitle, 256, 0.0, 256.0, m_nMinimiserBins,
                                        m_min_time_diff, m_max_time_diff);
  m_closure_tdiff_vs_sample1->SetContour(256);
  m_closure_tdiff_vs_sample1->SetXTitle("Sample number % 256");
  m_closure_tdiff_vs_sample1->SetYTitle("time difference / default sample widths");

  closureHistogramName = "closure_sample2_";
  closureHistogramName += ch_id;
  closureHistogramTitle = "Closure histogram for sample 2 for channel ";
  closureHistogramTitle += ch_id;
  m_closure_tdiff_vs_sample2 = new TH2D(closureHistogramName, closureHistogramTitle, 256, 0.0, 256.0, m_nMinimiserBins,
                                        m_min_time_diff, m_max_time_diff);
  m_closure_tdiff_vs_sample2->SetContour(256);
  m_closure_tdiff_vs_sample2->SetXTitle("Sample number % 256");
  m_closure_tdiff_vs_sample2->SetYTitle("time difference / default sample widths");

  closureHistogramName = "closure_time1_vs_time2_";
  closureHistogramName += ch_id;
  closureHistogramTitle = "Closure histogram: time1 vs time2 for channel ";
  closureHistogramTitle += ch_id;
  m_closure_time1_vs_time2 = new TH2D(closureHistogramName, closureHistogramTitle, 256 * 4, 0.0, 256.0, 256 * 4, 0.0, 256.0);
  m_closure_time1_vs_time2->SetContour(256);
  m_closure_time1_vs_time2->SetXTitle("time1 / default sample widths");
  m_closure_time1_vs_time2->SetYTitle("time2 / default sample widths");


  for (auto it(hitInfoVector.begin()); it != hitInfoVector.end(); ++it) {

    double nomDt  = 1.0;
    double winDt2 = nomDt * 255.0;
//    double winDt  = winDt2 / 2.0;

    int total = 256;

//    int time1_nwin = it->sample1 / total;
//    int time2_nwin = it->sample2 / total;
    double sample1 = fmod(it->sample1, total);
    double sample2 = fmod(it->sample2, total);
    double time1 = 0.;
    double time2 = 0.;
    double sample1_frac = modf(sample1, &sample1);
    double sample2_frac = modf(sample2, &sample2);

    int s1index(sample1);
    int s2index(sample2);
    s1index = s1index % 256;
    s2index = s2index % 256;

    time1 = m_dTval[s1index];
    time1 += sample1_frac * (m_dTval[s1index + 1] - m_dTval[s1index]);

    time2 = m_dTval[s2index];
    time2 += sample2_frac * (m_dTval[s2index + 1] - m_dTval[s2index]);

    double timeDifference(0);
    if ((time2 - time1) > 0) {timeDifference = (time2 - time1);}
    if ((time2 - time1) < 0) {timeDifference = ((winDt2 + time2) - time1);}

    m_closure_tdiff_vs_sample1->Fill(s1index, timeDifference);
    m_closure_tdiff_vs_sample2->Fill(s2index, timeDifference);
    m_closure_time1_vs_time2->Fill(time1, time2);


  }

  if (m_out_file) {

    m_out_file->cd("extra");
    m_closure_tdiff_vs_sample1->Write();
    m_closure_tdiff_vs_sample2->Write();
    m_closure_time1_vs_time2->Write();
    m_out_file->cd("");
  }

}
