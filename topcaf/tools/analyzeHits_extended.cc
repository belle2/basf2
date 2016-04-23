#include <TFile.h>
#include <TTree.h>
#include <TCut.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TChain.h>
#include <TMultiGraph.h>
#include <TH1F.h>
#include <TH2F.h>
#include <iostream>
#include <string>
#include <utility>
#include "TStyle.h"
#include "TSpectrum.h"
#include <cmath>
#include <ctime>
#include <vector>

using namespace std;
using namespace Belle2;

bool analyzeHits_extended(const char* filename, const char* outfilename = "hits.root", const char* trigpos = "",
                          const char* infofile = 0)
{

  vector<float> infovect;
  if (infofile) {
    ifstream in(infofile);
    string line;
    while (getline(in, line)) {
      infovect.push_back(atof(line.c_str()));
    }
  }

  string trigstring = string(trigpos);
  cout << trigstring << endl;

  clock_t t1 = clock();

  //detect fuji/tsukuba based on suffix
  //use cut to set trigger position

  TChain t("tree");
  t.Add(filename);
  int maxadc = 2000;

  TCut width_cut("TOPCAFDigits.m_width>3.&&TOPCAFDigits.m_width<20.");
  TCut width_cut2("TOPCAFDigits.m_width>3.&&TOPCAFDigits.m_width<10.");
  //TCut height_cut("TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<2048 && TOPCAFDigits.m_adc_height<900");
  //TCut height_cut2("TOPCAFDigits.m_adc_height>200&&TOPCAFDigits.m_adc_height<2048 && TOPCAFDigits.m_adc_height<900");
  TCut height_cut("TOPCAFDigits.m_adc_height>50");
  TCut height_cut2("TOPCAFDigits.m_adc_height>100&&TOPCAFDigits.m_adc_height<2048");
  TCut height_cut3("TOPCAFDigits.m_adc_height>150&&TOPCAFDigits.m_adc_height<2048");
  TCut qual_cut("TOPCAFDigits.m_flag>0&&TOPCAFDigits.m_corr_time!=0");
  //TCut qual_cut("TOPCAFDigits.m_flag>99 && TOPCAFDigits.m_flag<999 &&TOPCAFDigits.m_corr_time!=0"); //clean up the cal pulse?
  TCut optics_cut("TOPCAFDigits.m_width>3.&&TOPCAFDigits.m_width<10. && TOPCAFDigits.m_asic_ch !=7");
  TCut pmt_quality_cut;

  //Create a cut to remove "bad" ASICs
  //Previously done run-by-run, now done for overall
  if (infovect.size() > 3) {
    TString asiccut = "";
    //TOPCAFDigits.m_boardstack*16+TOPCAFDigits.m_asic_row*4+TOPCAFDigits.m_asic
    for (int ii = 3; ii < (int) infovect.size(); ii++) {
      int aI = (int) infovect.at(ii);
      if (ii == ((int) infovect.size() - 1)) {
        asiccut += Form("(TOPCAFDigits.m_boardstack==%i && TOPCAFDigits.m_asic_row==%i && TOPCAFDigits.m_asic==%i)", ((int) aI / 16),
                        (((int) aI / 4) % 4), (aI % 4));
      } else {
        asiccut += Form("(TOPCAFDigits.m_boardstack==%i && TOPCAFDigits.m_asic_row==%i && TOPCAFDigits.m_asic==%i) || ", ((int) aI / 16),
                        (((int) aI / 4) % 4), (aI % 4));
      }
    }
    cout << asiccut << endl;
    pmt_quality_cut = asiccut.Data();
  } else {
    pmt_quality_cut = "(TOPCAFDigits.m_pixel_id<-100)";
  }
  TCut cosmic_cut[10];

  TCut cut_loose = width_cut && height_cut && qual_cut;
  TCut cut_tight = width_cut2 && height_cut2 && qual_cut && !pmt_quality_cut;
  TCut cut_optics = width_cut2 && height_cut3 && qual_cut && optics_cut && !pmt_quality_cut;


  //Define the cuts here using a TSpectrum to search first
  TH2F* ch_time_qual = new TH2F("ch_time_qual", "time vs pixel (quality cuts)", 512, 0., 512., 40000, -2000., 2000.);
  TH1D* ch_time_proj_qual = new TH1D("ch_time_proj_qual", "time projection (qual cuts)", 40000, -2000., 2000.);
  //t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", width_cut && qual_cut && height_cut2 && !pmt_quality_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", cut_tight);
  ch_time_proj_qual = ch_time_qual->ProjectionY();
  ch_time_proj_qual->Rebin(10);

  const int npeaks = 1;
  TH1F* roi[3];
  for (int ii = 0; ii < 3; ii++) {
    roi[ii] = (TH1F*)ch_time_proj_qual->Clone();
    roi[ii]->Rebin(2);
  }

  const int flag_lab = (((strstr(filename, "cpr31") != NULL)) || ((strstr(filename, "cpr32") != NULL))) ;
  //0=tsukuba, 1=fuji
  if (flag_lab == 1) {
    if (trigstring == "mirror") {
      //roi[0]->GetXaxis()->SetRangeUser(-1220, -1180);
      roi[0]->GetXaxis()->SetRangeUser(-1240, -1170);
    }
    if (trigstring == "midbar") {
      //roi[0]->GetXaxis()->SetRangeUser(-1250, -1220);
      roi[0]->GetXaxis()->SetRangeUser(-1270, -1200);
    }
    if (trigstring == "prism") {
      //roi[0]->GetXaxis()->SetRangeUser(-1240, -1170);
      roi[0]->GetXaxis()->SetRangeUser(-1260, -1160);
    }
  } else {
    //This section does not matter if using Umberto's suggested simplification below for Tsukuba
    if (trigstring == "mirror") {
      roi[0]->GetXaxis()->SetRangeUser(-500, -100);
    }
    if (trigstring == "midbar") {
      roi[0]->GetXaxis()->SetRangeUser(-475, -445);
    }
    if (trigstring == "prism") {
      roi[0]->GetXaxis()->SetRangeUser(-500, -100);
    }
  }

  TSpectrum* s0 = new TSpectrum(npeaks);
  int nfound0 = s0->Search(roi[0], 1, "", 0.01);
  Double_t* xpeaks0 = s0->GetPositionX();

  //Use Umberto's suggestion for simplification
  //At Tsukuba, find the maximum bin as the central peak, no matter what type of run it is
  if (flag_lab != 1) {
    roi[0]->GetXaxis()->SetRangeUser(-500, -10);
    xpeaks0[0] = roi[0]->GetBinCenter(roi[0]->GetMaximumBin());
  }

  if (trigstring == "mirror") {
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0] + 90 - 25, xpeaks0[0] + 90 + 25);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0] - 42.5 - 22.5, xpeaks0[0] - 42.5 + 22.5);
  }
  if (trigstring == "midbar") {
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0] + 77.5 - 17.5, xpeaks0[0] + 77.5 + 17.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0] + 162 - 20, xpeaks0[0] + 162 + 20);
  }
  if (trigstring == "prism") {
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0] + 77.5 - 32.5, xpeaks0[0] + 77.5 + 32.5);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0] - 37.5 - 22.5, xpeaks0[0] - 37.5 + 22.5);
  }
  if (trigstring == "laser") {
    roi[1]->GetXaxis()->SetRangeUser(xpeaks0[0] + 103.5 - 14, xpeaks0[0] + 103.5 + 14);
    roi[2]->GetXaxis()->SetRangeUser(xpeaks0[0] - 37.5 - 22.5, xpeaks0[0] - 37.5 + 22.5);
  }


  TSpectrum* s1 = new TSpectrum(npeaks);
  int nfound1 = s1->Search(roi[1], 1, "", 0.01);
  Double_t* xpeaks1 = s1->GetPositionX();
  TSpectrum* s2 = new TSpectrum(npeaks);
  int nfound2 = s2->Search(roi[2], 1, "", 0.01);
  Double_t* xpeaks2 = s2->GetPositionX();
  cout << nfound0 << nfound1 << nfound2 << endl;

  //Previously done run-by-run, now for ensemble of runs
  if (infofile) {
    if (infovect.at(0) != 0) {
      xpeaks0[0] = infovect.at(0);
      xpeaks1[0] = infovect.at(1);
      xpeaks2[0] = infovect.at(2);
    }
  }

  cout << xpeaks0[0] << " " << xpeaks1[0] << " " << xpeaks2[0] << endl;

  if (trigstring == "mirror") {
    cosmic_cut[0] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks0[0] - 13, xpeaks0[0] + 13));
    cosmic_cut[1] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks1[0] - 16, xpeaks1[0] + 16));
    cosmic_cut[2] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks2[0] - 12, xpeaks2[0] + 12));
  }
  if (trigstring == "midbar" || trigstring == "prism") {
    cosmic_cut[0] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks0[0] - 9, xpeaks0[0] + 9));
    cosmic_cut[1] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks1[0] - 13, xpeaks1[0] + 13));
    cosmic_cut[2] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks2[0] - 12, xpeaks2[0] + 12));
  }
  if (trigstring == "laser") {
    //cosmic_cut[0] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks0[0] - 9, xpeaks0[0] + 9));
    //cosmic_cut[1] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks1[0] - 14, xpeaks1[0] + 14));
    //Redone for installed modules
    cosmic_cut[0] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks0[0] - 3, xpeaks0[0] + 3));
    cosmic_cut[1] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks0[0] + 80 - 7, xpeaks0[0] + 80 + 7));
    cosmic_cut[2] = (Form("TOPCAFDigits.m_time>%f && TOPCAFDigits.m_time<%f", xpeaks2[0] - 12, xpeaks2[0] + 12));
  }


  TFile fout(outfilename, "RECREATE");

  //Define the histograms
  TH1F* scrods = new TH1F("scrods", "scrod id numbers (no cuts)", 120, 0., 120.);
  scrods->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths = new TH2F("ch_widths", "hit width vs pixel (quality cuts)", 512, 0., 512., 1000, 0., 100.);
  ch_widths->GetXaxis()->SetTitle("pixel");
  ch_widths->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights = new TH2F("ch_heights", "hit height vs pixel (quality cuts)", 512, 0., 512., 200, -2048., 2048.);
  ch_heights->GetXaxis()->SetTitle("pixel");
  ch_heights->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* ch_adcint = new TH2F("ch_adcint", "adcint vs pixel (quality cuts)", 512, 0., 512., 500, 0, 10000);
  ch_adcint->GetXaxis()->SetTitle("pixel");
  ch_adcint->GetYaxis()->SetTitle("adc int");

  TH2F* width_time = new TH2F("width_time", "widths vs times (quality cuts)", 2500, 0., 2500., 1000, 0., 100.);
  width_time->GetXaxis()->SetTitle("hit tdc bin");
  width_time->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time = new TH2F("height_time", "heights vs times (quality cuts)", 2500, 0., 2500., 1000, -2000., 2000.);
  height_time->GetXaxis()->SetTitle("hit tdc bin");
  height_time->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time = new TH2F("adcint_time", "adcint vs times (quality cuts)", 2500, 0., 2500., 500, 0, 10000);
  adcint_time->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* width_time_corr = new TH2F("width_time_corr", "widths vs corr_times (quality cuts)", 3000, -2000., 1000., 500, 0., 100.);
  width_time_corr->GetXaxis()->SetTitle("hit tdc bin");
  width_time_corr->GetYaxis()->SetTitle("hit width [corr_time bin]");

  TH2F* height_time_corr = new TH2F("height_time_corr", "heights vs corr_times (quality cuts)", 3000, -2000., 1000., 500, 0., 2000.);
  height_time_corr->GetXaxis()->SetTitle("hit tdc bin");
  height_time_corr->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_corr = new TH2F("adcint_time_corr", "adcint vs corr_times (quality cuts)", 3000, -2000., 1000., 500, 0, 10000);
  adcint_time_corr->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_corr->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* ch_time = new TH2F("ch_time", "time vs pixel", 512, 0., 512., 40000, -2000., 2000.);
  ch_time->GetXaxis()->SetTitle("pixel");
  ch_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH1D* ch_time_proj = new TH1D("ch_time_proj", "time projection", 40000, -2000., 2000.);
  ch_time_proj->GetXaxis()->SetTitle("time");

  //TH2F* ch_time_qual = new TH2F("ch_time_qual", "time vs pixel (quality cuts)", 512, 0., 512., 40000, -2000., 2000.);
  ch_time->GetXaxis()->SetTitle("pixel");
  ch_time->GetYaxis()->SetTitle("hit time [time bin]");

  //TH1D* ch_time_proj_qual = new TH1D("ch_time_proj_qual", "time projection (qual cuts)", 40000, -2000., 2000.);
  ch_time_proj_qual->GetXaxis()->SetTitle("time");

  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel (quality cuts)", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin->GetXaxis()->SetTitle("pixel");
  ch_tdcbin->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time = new TH2F("tdcbin_time", "hit time vs tdc bin (quality cuts)", 1500, 0., (double)maxadc, 4000, -2000., 2000.);
  tdcbin_time->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* gainxy = new TH2F("gainxy", "adcint vs pmt xy position (good events)", 64, 0., 64., 8, 0., 8.);
  gainxy->GetXaxis()->SetTitle("pmt x");
  gainxy->GetYaxis()->SetTitle("pmt y");
  gainxy->SetStats(0);

  TH2F* pulser_height_width = new TH2F("pulser_height_width", "pulser signal: height VS width", 200, 0., 20., 1000, -1000., 0.);
  pulser_height_width->GetXaxis()->SetTitle(" width [ns]");
  pulser_height_width->GetYaxis()->SetTitle(" amplitude [acd counts]");

  TH1F* channel_occupancy = new TH1F("channel_occupancy", "hits in each pixel", 512, 0., 512.);
  channel_occupancy->GetXaxis()->SetTitle("pixel");
  channel_occupancy->GetYaxis()->SetTitle("counts");

  TH1F* digitsperchannel = new TH1F("digitsperchannel", "TOPCAF digits per channel", 8, 0, 8);
  digitsperchannel->GetXaxis()->SetTitle("channel");


  TH1F* asic_occupancy[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c] = new TH1F(Form("asic_occupancy%d", c),
                                 Form("boardstack %d hits in each asic (no cuts)", c), 350, 0., 350.);
    asic_occupancy[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy[c]->GetYaxis()->SetTitle("counts");
  }
  TH1F* asic_occupancy_all = new TH1F("asic_occupancy_all", "asic occupancy all", 64, 0, 64.);
  TH1F* asic_cold = new TH1F("asic_cold", "cold asics", 64, 0, 64.);
  TH1F* asic_warm = new TH1F("asic_warm", "warm asics", 64, 0, 64.);
  TH1F* asic_hott = new TH1F("asic_hott", "hot asics", 64, 0, 64.);

  TH1F* evtnum[10];
  TH1F* evtnumq[10];
  TH1F* nhit[10];
  TH1F* nhitq[10];
  TH2F* pmt_xy[10];
  TH2F* pmt_xyq[10];
  TH1F* adcint[10];
  for (int ii = 0; ii < 10; ii++) {
    char n_evt[20], n_hit[20], n_pmt[20], n_adcint[20];
    sprintf(n_evt, "evtnum%d", ii);
    sprintf(n_hit, "nhit%d", ii);
    evtnum[ii] = new TH1F(n_evt, n_evt, 1000, 0, 16000);
    nhit[ii] = new TH1F(n_hit, n_hit, 100, 0, 100);
    sprintf(n_evt, "evtnumq%d", ii);
    sprintf(n_hit, "nhitq%d", ii);
    evtnumq[ii] = new TH1F(n_evt, n_evt, 1000, 0, 16000);
    nhitq[ii] = new TH1F(n_hit, n_hit, 100, 0, 100);
    sprintf(n_pmt, "pmt_xy%d", ii);
    pmt_xy[ii] = new TH2F(n_pmt, n_pmt, 64, 0., 64., 8, 0., 8.);
    pmt_xy[ii]->SetStats(0);
    sprintf(n_pmt, "pmt_xyq%d", ii);
    pmt_xyq[ii] = new TH2F(n_pmt, n_pmt, 64, 0., 64., 8, 0., 8.);
    pmt_xyq[ii]->SetStats(0);
    sprintf(n_adcint, "adcint%d", ii);
    adcint[ii] = new TH1F(n_adcint, n_adcint, 1000, 0, 10000);
  }

  TH1F* scrods_tight = new TH1F("scrods_tight", "scrod id numbers (tight cuts)", 120, 0., 120.);
  scrods_tight->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths_tight = new TH2F("ch_widths_tight", "hit width vs pixel (tight cuts)", 512, 0., 512., 1000, 0., 100.);
  ch_widths_tight->GetXaxis()->SetTitle("pixel");
  ch_widths_tight->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights_tight = new TH2F("ch_heights_tight", "hit height vs pixel (tight cuts)", 512, 0., 512., 200, -2048., 2048.);
  ch_heights_tight->GetXaxis()->SetTitle("pixel");
  ch_heights_tight->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* ch_adcint_tight = new TH2F("ch_adcint_tight", "adcint vs pixel (tight cuts)", 512, 0., 512., 500, 0, 10000);
  ch_adcint_tight->GetXaxis()->SetTitle("pixel");
  ch_adcint_tight->GetYaxis()->SetTitle("adc int");

  TH2F* width_time_tight = new TH2F("width_time_tight", "widths vs times (tight cuts)", 2500, 0., 2500., 1000, 0., 100.);
  width_time_tight->GetXaxis()->SetTitle("hit tdc bin");
  width_time_tight->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time_tight = new TH2F("height_time_tight", "heights vs times (tight cuts)", 2500, 0., 2500., 1000, -2000., 2000.);
  height_time_tight->GetXaxis()->SetTitle("hit tdc bin");
  height_time_tight->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_tight = new TH2F("adcint_time_tight", "adcint vs times (tight cuts)", 2500, 0., 2500., 500, 0, 10000);
  adcint_time_tight->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_tight->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* width_time_corr_tight = new TH2F("width_time_corr_tight", "widths vs corr_times (tight cuts)", 3000, -2000., 1000., 500, 0.,
                                         100.);
  width_time_corr_tight->GetXaxis()->SetTitle("hit tdc bin");
  width_time_corr_tight->GetYaxis()->SetTitle("hit width [corr_time bin]");

  TH2F* height_time_corr_tight = new TH2F("height_time_corr_tight", "heights vs corr_times (tight cuts)", 3000, -2000., 1000., 500,
                                          0., 2000.);
  height_time_corr_tight->GetXaxis()->SetTitle("hit tdc bin");
  height_time_corr_tight->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_corr_tight = new TH2F("adcint_time_corr_tight", "adcint vs corr_times (tight cuts)", 3000, -2000., 1000., 500, 0,
                                          10000);
  adcint_time_corr_tight->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_corr_tight->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* ch_time_tight = new TH2F("ch_time_tight", "time vs pixel", 512, 0., 512., 40000, -2000., 2000.);
  ch_time_tight->GetXaxis()->SetTitle("pixel");
  ch_time_tight->GetYaxis()->SetTitle("hit time [time bin]");

  TH1D* ch_time_proj_tight = new TH1D("ch_time_proj_tight", "time projection", 40000, -2000., 2000.);
  ch_time_proj_tight->GetXaxis()->SetTitle("time");

  TH2F* ch_tdcbin_tight = new TH2F("ch_tdcbin_tight", "tdc bin vs channel (tight cuts)", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin_tight->GetXaxis()->SetTitle("pixel");
  ch_tdcbin_tight->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time_tight = new TH2F("tdcbin_time_tight", "hit time vs tdc bin (tight cuts)", 1500, 0., (double)maxadc, 4000, -2000.,
                                     2000.);
  tdcbin_time_tight->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time_tight->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* gainxy_tight = new TH2F("gainxy_tight", "adcint vs pmt xy position (tight cuts)", 64, 0., 64., 8, 0., 8.);
  gainxy_tight->GetXaxis()->SetTitle("pmt x");
  gainxy_tight->GetYaxis()->SetTitle("pmt y");
  gainxy_tight->SetStats(0);

  TH2F* pulser_height_width_tight = new TH2F("pulser_height_width_tight", "pulser signal: height VS width (tight cuts)", 200, 0., 20.,
                                             1000, -1000., 0.);
  pulser_height_width_tight->GetXaxis()->SetTitle(" width [ns]");
  pulser_height_width_tight->GetYaxis()->SetTitle(" amplitude [acd counts]");

  TH1F* channel_occupancy_tight = new TH1F("channel_occupancy_tight", "hits in each pixel (tight cuts)", 512, 0., 512.);
  channel_occupancy_tight->GetXaxis()->SetTitle("pixel");
  channel_occupancy_tight->GetYaxis()->SetTitle("counts");

  TH1F* digitsperchannel_tight = new TH1F("digitsperchannel_tight", "TOPCAF digits per channel (tight cuts)", 8, 0, 8);
  digitsperchannel_tight->GetXaxis()->SetTitle("channel");

  TH1F* asic_occupancy_tight[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy_tight[c] = new TH1F(Form("asic_occupancy_tight%d", c),
                                       Form("boardstack %d hits in each asic (tight cuts)", c), 350, 0., 350.);
    asic_occupancy_tight[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy_tight[c]->GetYaxis()->SetTitle("counts");
  }
  TH1F* asic_occupancy_all_tight = new TH1F("asic_occupancy_all_tight", "asic occupancy all (tight cuts)", 64, 0, 64.);
  TH1F* asic_cold_tight = new TH1F("asic_cold_tight", "cold asics (tight cuts)", 64, 0, 64.);
  TH1F* asic_warm_tight = new TH1F("asic_warm_tight", "warm asics (tight cuts)", 64, 0, 64.);
  TH1F* asic_hott_tight = new TH1F("asic_hott_tight", "hot asics (tight cuts)", 64, 0, 64.);

  TH1F* evtnum_tight[10];
  TH1F* nhit_tight[10];
  TH2F* pmt_xy_tight[10];
  TH1F* adcint_tight[10];
  for (int ii = 0; ii < 10; ii++) {
    char n_evt_tight[20], n_hit_tight[20], n_pmt_tight[20], n_adcint_tight[20];
    sprintf(n_evt_tight, "evtnum_tight%d", ii);
    sprintf(n_hit_tight, "nhit_tight%d", ii);
    evtnum_tight[ii] = new TH1F(n_evt_tight, n_evt_tight, 1000, 0, 16000);
    nhit_tight[ii] = new TH1F(n_hit_tight, n_hit_tight, 100, 0, 100);
    sprintf(n_pmt_tight, "pmt_xy_tight%d", ii);
    pmt_xy_tight[ii] = new TH2F(n_pmt_tight, n_pmt_tight, 64, 0., 64., 8, 0., 8.);
    pmt_xy_tight[ii]->SetStats(0);
    sprintf(n_adcint_tight, "adcint_tight%d", ii);
    adcint_tight[ii] = new TH1F(n_adcint_tight, n_adcint_tight, 1000, 0, 10000);
  }

  TH1F* scrods_optics = new TH1F("scrods_optics", "scrod id numbers (optics cuts)", 120, 0., 120.);
  scrods_optics->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths_optics = new TH2F("ch_widths_optics", "hit width vs pixel (optics cuts)", 512, 0., 512., 1000, 0., 100.);
  ch_widths_optics->GetXaxis()->SetTitle("pixel");
  ch_widths_optics->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights_optics = new TH2F("ch_heights_optics", "hit height vs pixel (optics cuts)", 512, 0., 512., 200, -2048., 2048.);
  ch_heights_optics->GetXaxis()->SetTitle("pixel");
  ch_heights_optics->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* ch_adcint_optics = new TH2F("ch_adcint_optics", "adcint vs pixel (optics cuts)", 512, 0., 512., 500, 0, 10000);
  ch_adcint_optics->GetXaxis()->SetTitle("pixel");
  ch_adcint_optics->GetYaxis()->SetTitle("adc int");

  TH2F* width_time_optics = new TH2F("width_time_optics", "widths vs times (optics cuts)", 2500, 0., 2500., 1000, 0., 100.);
  width_time_optics->GetXaxis()->SetTitle("hit tdc bin");
  width_time_optics->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time_optics = new TH2F("height_time_optics", "heights vs times (optics cuts)", 2500, 0., 2500., 1000, -2000., 2000.);
  height_time_optics->GetXaxis()->SetTitle("hit tdc bin");
  height_time_optics->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_optics = new TH2F("adcint_time_optics", "adcint vs times (optics cuts)", 2500, 0., 2500., 500, 0, 10000);
  adcint_time_optics->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_optics->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* width_time_corr_optics = new TH2F("width_time_corr_optics", "widths vs corr_times (optics cuts)", 3000, -2000., 1000., 500,
                                          0., 100.);
  width_time_corr_optics->GetXaxis()->SetTitle("hit tdc bin");
  width_time_corr_optics->GetYaxis()->SetTitle("hit width [corr_time bin]");

  TH2F* height_time_corr_optics = new TH2F("height_time_corr_optics", "heights vs corr_times (optics cuts)", 3000, -2000., 1000., 500,
                                           0., 2000.);
  height_time_corr_optics->GetXaxis()->SetTitle("hit tdc bin");
  height_time_corr_optics->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_corr_optics = new TH2F("adcint_time_corr_optics", "adcint vs corr_times (optics cuts)", 3000, -2000., 1000., 500,
                                           0, 10000);
  adcint_time_corr_optics->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_corr_optics->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* ch_time_optics = new TH2F("ch_time_optics", "time vs pixel", 512, 0., 512., 40000, -2000., 2000.);
  ch_time_optics->GetXaxis()->SetTitle("pixel");
  ch_time_optics->GetYaxis()->SetTitle("hit time [time bin]");

  TH1D* ch_time_proj_optics = new TH1D("ch_time_proj_optics", "time projection", 40000, -2000., 2000.);
  ch_time_proj_optics->GetXaxis()->SetTitle("time");

  TH2F* ch_tdcbin_optics = new TH2F("ch_tdcbin_optics", "tdc bin vs channel (optics cuts)", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin_optics->GetXaxis()->SetTitle("pixel");
  ch_tdcbin_optics->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time_optics = new TH2F("tdcbin_time_optics", "hit time vs tdc bin (optics cuts)", 1500, 0., (double)maxadc, 4000,
                                      -2000., 2000.);
  tdcbin_time_optics->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time_optics->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* gainxy_optics = new TH2F("gainxy_optics", "adcint vs pmt xy position (optics cuts)", 64, 0., 64., 8, 0., 8.);
  gainxy_optics->GetXaxis()->SetTitle("pmt x");
  gainxy_optics->GetYaxis()->SetTitle("pmt y");
  gainxy_optics->SetStats(0);

  TH2F* pulser_height_width_optics = new TH2F("pulser_height_width_optics", "pulser signal: height VS width (optics cuts)", 200, 0.,
                                              20., 1000, -1000., 0.);
  pulser_height_width_optics->GetXaxis()->SetTitle(" width [ns]");
  pulser_height_width_optics->GetYaxis()->SetTitle(" amplitude [acd counts]");

  TH1F* channel_occupancy_optics = new TH1F("channel_occupancy_optics", "hits in each pixel (optics cuts)", 512, 0., 512.);
  channel_occupancy_optics->GetXaxis()->SetTitle("pixel");
  channel_occupancy_optics->GetYaxis()->SetTitle("counts");

  TH1F* digitsperchannel_optics = new TH1F("digitsperchannel_optics", "TOPCAF digits per channel (optics cuts)", 8, 0, 8);
  digitsperchannel_optics->GetXaxis()->SetTitle("channel");

  TH1F* asic_occupancy_optics[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy_optics[c] = new TH1F(Form("asic_occupancy_optics%d", c),
                                        Form("boardstack %d hits in each asic (optics cuts)", c), 350, 0., 350.);
    asic_occupancy_optics[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy_optics[c]->GetYaxis()->SetTitle("counts");
  }
  TH1F* asic_occupancy_all_optics = new TH1F("asic_occupancy_all_optics", "asic occupancy all (optics cuts)", 64, 0, 64.);
  TH1F* asic_cold_optics = new TH1F("asic_cold_optics", "cold asics (optics cuts)", 64, 0, 64.);
  TH1F* asic_warm_optics = new TH1F("asic_warm_optics", "warm asics (optics cuts)", 64, 0, 64.);
  TH1F* asic_hott_optics = new TH1F("asic_hott_optics", "hot asics (optics cuts)", 64, 0, 64.);

  TH1F* evtnum_optics[10];
  TH1F* nhit_optics[10];
  TH2F* pmt_xy_optics[10];
  TH1F* adcint_optics[10];
  for (int ii = 0; ii < 10; ii++) {
    char n_evt_optics[20], n_hit_optics[20], n_pmt_optics[20], n_adcint_optics[20];
    sprintf(n_evt_optics, "evtnum_optics%d", ii);
    sprintf(n_hit_optics, "nhit_optics%d", ii);
    evtnum_optics[ii] = new TH1F(n_evt_optics, n_evt_optics, 1000, 0, 16000);
    nhit_optics[ii] = new TH1F(n_hit_optics, n_hit_optics, 100, 0, 100);
    sprintf(n_pmt_optics, "pmt_xy_optics%d", ii);
    pmt_xy_optics[ii] = new TH2F(n_pmt_optics, n_pmt_optics, 64, 0., 64., 8, 0., 8.);
    pmt_xy_optics[ii]->SetStats(0);
    sprintf(n_adcint_optics, "adcint_optics%d", ii);
    adcint_optics[ii] = new TH1F(n_adcint_optics, n_adcint_optics, 1000, 0, 10000);
  }


  clock_t t2 = clock();
  cout << "defs" << (t2 - t1) << endl;
  t1 = t2;

  //Define cosmic_cut regions
  //0: prompt
  //1: reflected
  //2: reflected 2
  //3: laser
  //4: wideband
  //5: sideband

  //const int flag_lab = strstr(filename, "cpr3015") != NULL;
  //0=fuji, 1=tsukuba
  //cout<<flag_lab<<endl;
  if (flag_lab == 1) {
    //FUJI
    cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
    cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
    cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
  } else {
    //TSUKUBA
    cosmic_cut[3] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<1");
    cosmic_cut[4] = ("TOPCAFDigits.m_time>-550&&TOPCAFDigits.m_time<-300");
    cosmic_cut[5] = ("TOPCAFDigits.m_time>-10&&TOPCAFDigits.m_time<10");
    if (trigstring == "laser") {
      cout << "laser" << endl;
      //cosmic_cut[0] = ("TOPCAFDigits.m_time>355&&TOPCAFDigits.m_time<375");
      //cosmic_cut[1] = ("TOPCAFDigits.m_time>115&&TOPCAFDigits.m_time<130");
      //cosmic_cut[2] = ("TOPCAFDigits.m_time>60&&TOPCAFDigits.m_time<80");
      cosmic_cut[3] = ("TOPCAFDigits.m_time>30&&TOPCAFDigits.m_time<50");
      cosmic_cut[4] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<10");
      cosmic_cut[5] = ("TOPCAFDigits.m_time>85&&TOPCAFDigits.m_time<110");
    }
  }

  t2 = clock();
  cout << "ready " << (t2 - t1) << endl;
  t1 = t2;

  for (int ii = 0; ii < 8; ii++) {
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnum%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnumq%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut2 && !pmt_quality_cut);
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnum_tight%d", ii), cut_tight && cosmic_cut[ii]);
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnum_optics%d", ii), cut_optics && cosmic_cut[ii]);
    TString ts1 = "(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>";
    TString ts2 = Form("pmt_xy%d", ii);
    ts1 = ts1 + ts2;
    t.Draw(ts1, width_cut && qual_cut && cosmic_cut[ii] && height_cut);
    pmt_xy[ii]->SetOption("COLZ");
    pmt_xy[ii]->Draw();

    TString ts1q = "(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>";
    TString ts2q = Form("pmt_xyq%d", ii);
    ts1q = ts1q + ts2q;
    t.Draw(ts1q, width_cut && qual_cut && cosmic_cut[ii] && height_cut2 && !pmt_quality_cut);
    pmt_xyq[ii]->SetOption("COLZ");
    pmt_xyq[ii]->Draw();

    ts1q = "(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>";
    ts2q = Form("pmt_xy_tight%d", ii);
    ts1q = ts1q + ts2q;
    t.Draw(ts1q, cosmic_cut[ii] && cut_tight);
    pmt_xy_tight[ii]->SetOption("COLZ");
    pmt_xy_tight[ii]->Draw();

    ts1q = "(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>";
    ts2q = Form("pmt_xy_optics%d", ii);
    ts1q = ts1q + ts2q;
    t.Draw(ts1q, cosmic_cut[ii] && cut_optics);
    pmt_xy_optics[ii]->SetOption("COLZ");
    pmt_xy_optics[ii]->Draw();

    t.Draw(Form("(TOPCAFDigits.m_q)>>adcint%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    t.Draw(Form("(TOPCAFDigits.m_q)>>adcint_tight%d", ii), cosmic_cut[ii] && cut_tight);
    t.Draw(Form("(TOPCAFDigits.m_q)>>adcint_optics%d", ii), cosmic_cut[ii] && cut_optics);

    for (int i = 0; i < 1000; i++) {
      nhit[ii]->Fill(evtnum[ii]->GetBinContent(i));
      nhitq[ii]->Fill(evtnumq[ii]->GetBinContent(i));
      nhit_tight[ii]->Fill(evtnum_tight[ii]->GetBinContent(i));
      nhit_optics[ii]->Fill(evtnum_optics[ii]->GetBinContent(i));
    }
  }

  TH1F* time_by_asic[64], *time_by_asic_tight[64], *time_by_asic_optics[64];
  for (int ii = 0; ii < 64; ii++) {
    char n_asic[20];
    sprintf(n_asic, "asic_time%i", ii);
    time_by_asic[ii] = new TH1F(n_asic, n_asic, 4000, -2000, 2000);
    sprintf(n_asic, "asic_time_tight%i", ii);
    time_by_asic_tight[ii] = new TH1F(n_asic, n_asic, 4000, -2000, 2000);
    sprintf(n_asic, "asic_time_optics%i", ii);
    time_by_asic_optics[ii] = new TH1F(n_asic, n_asic, 4000, -2000, 2000);
  }



  //Loop over all of the ASICs and fill the time projection plot
  TString asiccut_tmp = "";
  TCut asiccut_cut;
  //TOPCAFDigits.m_boardstack*16+TOPCAFDigits.m_asic_row*4+TOPCAFDigits.m_asic
  for (int ii = 0; ii < 64; ii++) {
    asiccut_tmp = Form("(TOPCAFDigits.m_boardstack==%i && TOPCAFDigits.m_asic_row==%i && TOPCAFDigits.m_asic==%i)", ((int) ii / 16),
                       (((int) ii / 4) % 4), (ii % 4));
    asiccut_cut = asiccut_tmp.Data();
    t.Draw(Form("TOPCAFDigits.m_time>>asic_time%i", ii), width_cut && qual_cut && height_cut2 && asiccut_cut);
    t.Draw(Form("TOPCAFDigits.m_time>>asic_time_tight%i", ii), cut_tight && asiccut_cut);
    t.Draw(Form("TOPCAFDigits.m_time>>asic_time_optics%i", ii), cut_optics && asiccut_cut);
  }


  t2 = clock();
  cout << "looped " << (t2 - t1) << endl;
  t1 = t2;

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods");
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights", width_cut && qual_cut);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_pixel_id>>ch_adcint", qual_cut && width_cut && height_cut && !pmt_quality_cut
         && cosmic_cut[0]);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time", qual_cut && width_cut);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_tdc_bin>>adcint_time", qual_cut && width_cut && height_cut && !pmt_quality_cut);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_time>>width_time_corr", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_time>>height_time_corr", qual_cut && width_cut);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_time>>adcint_time_corr", qual_cut && width_cut && height_cut && !pmt_quality_cut);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time", width_cut && qual_cut && height_cut && !pmt_quality_cut);
  t.Draw("TOPCAFDigits.m_pixel_id>>channel_occupancy", width_cut && qual_cut && cosmic_cut[0] && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_width>>pulser_height_width",
         "TOPCAFDigits.m_adc_height< 0 && TOPCAFDigits.m_asic_ch==7");

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_pixel_id>>ch_adcint_tight", cosmic_cut[0] && cut_tight);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_tdc_bin>>adcint_time_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_time>>width_time_corr_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_time>>height_time_corr_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_time>>adcint_time_corr_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_tight", cut_tight);
  t.Draw("TOPCAFDigits.m_pixel_id>>channel_occupancy_tight", cosmic_cut[0] && cut_tight);

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_pixel_id>>ch_adcint_optics", cosmic_cut[0] && cut_optics);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_tdc_bin>>adcint_time_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_time>>width_time_corr_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_time>>height_time_corr_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_time>>adcint_time_corr_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_optics", cut_optics);
  t.Draw("TOPCAFDigits.m_pixel_id>>channel_occupancy_optics", cosmic_cut[0] && cut_optics);

  t2 = clock();
  cout << "drew " << (t2 - t1) << endl;
  t1 = t2;

  /*
  TH1F* tgain=new TH1F("tgain","tgain",100,0,10000);
  for(int c =1 ; c < 513; c++){
    //apply cuts for each pixel, get mean gain
    TCut temp_pmt(Form("TOPCAFDigits.m_pixel_id==%d",c));
    t.Draw("TOPCAFDigits.m_q>>tgain",width_cut && qual_cut && cosmic_cut[0] && height_cut && temp_pmt);
    //Ignore the zero bin
    gainxy->SetBinContent(int((c-1)%64)+1,int((c-1)/64)+1,tgain->GetMean(1)*tgain->GetEntries()/(tgain->GetEntries()-tgain->GetBinContent(1)));
    t2 = clock();
    cout<<(t2-t1)<<endl;
    t1=t2;
  }
  gainxy->SetOption("COLZ");
  gainxy->Draw();
  */

  t2 = clock();
  cout << "gain " << (t2 - t1) << endl;
  t1 = t2;

  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy%d", c),
           Form("TOPCAFDigits.m_boardstack==%d", c));
  }
  t.Draw("(TOPCAFDigits.m_boardstack*16+TOPCAFDigits.m_asic_row*4+TOPCAFDigits.m_asic)>>asic_occupancy_all", width_cut && qual_cut
         && (cosmic_cut[0]) && height_cut && "TOPCAFDigits.m_asic_ch!=7");

  Float_t a_cut = (asic_occupancy_all->GetEntries()) / 64;
  for (int i = 1; i < 65; i++) {
    if (asic_occupancy_all->GetBinContent(i) > (a_cut)) asic_warm->AddBinContent(i);
    if (asic_occupancy_all->GetBinContent(i) > (a_cut * 1.5)) asic_hott->AddBinContent(i);
    if (asic_occupancy_all->GetBinContent(i) < (a_cut * 0.25)) asic_cold->AddBinContent(i);
  }
  t.Draw("TOPCAFDigits.m_asic_ch>>digitsperchannel", height_cut);

  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy_tight%d", c), cut_tight
           && Form("TOPCAFDigits.m_boardstack==%d", c));
  }
  t.Draw("(TOPCAFDigits.m_boardstack*16+TOPCAFDigits.m_asic_row*4+TOPCAFDigits.m_asic)>>asic_occupancy_all_tight", cut_tight
         && (cosmic_cut[0]) && "TOPCAFDigits.m_asic_ch!=7");

  a_cut = (asic_occupancy_all_tight->GetEntries()) / 64;
  for (int i = 1; i < 65; i++) {
    if (asic_occupancy_all_tight->GetBinContent(i) > (a_cut)) asic_warm_tight->AddBinContent(i);
    if (asic_occupancy_all_tight->GetBinContent(i) > (a_cut * 1.5)) asic_hott_tight->AddBinContent(i);
    if (asic_occupancy_all_tight->GetBinContent(i) < (a_cut * 0.25)) asic_cold_tight->AddBinContent(i);
  }
  t.Draw("TOPCAFDigits.m_asic_ch>>digitsperchannel_tight", cut_tight);

  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy_optics%d", c), cut_optics
           && Form("TOPCAFDigits.m_boardstack==%d", c));
  }
  t.Draw("(TOPCAFDigits.m_boardstack*16+TOPCAFDigits.m_asic_row*4+TOPCAFDigits.m_asic)>>asic_occupancy_all_optics", cut_optics
         && (cosmic_cut[0]) && "TOPCAFDigits.m_asic_ch!=7");

  a_cut = (asic_occupancy_all_optics->GetEntries()) / 64;
  for (int i = 1; i < 65; i++) {
    if (asic_occupancy_all_optics->GetBinContent(i) > (a_cut)) asic_warm_optics->AddBinContent(i);
    if (asic_occupancy_all_optics->GetBinContent(i) > (a_cut * 1.5)) asic_hott_optics->AddBinContent(i);
    if (asic_occupancy_all_optics->GetBinContent(i) < (a_cut * 0.25)) asic_cold_optics->AddBinContent(i);
  }
  t.Draw("TOPCAFDigits.m_asic_ch>>digitsperchannel_optics", cut_optics);

  ch_time_proj = ch_time->ProjectionY();
  ch_time_proj->Rebin(10);
  ch_time_proj_tight = ch_time_tight->ProjectionY();
  ch_time_proj_tight->Rebin(10);
  ch_time_proj_optics = ch_time_optics->ProjectionY();
  ch_time_proj_optics->Rebin(10);

  fout.cd();
  scrods->Write();
  ch_widths->Write();
  ch_heights->Write();
  width_time->Write();
  height_time->Write();
  adcint_time->Write();
  width_time_corr->Write();
  height_time_corr->Write();
  adcint_time_corr->Write();
  ch_time->Write();
  ch_time_qual->Write();
  ch_tdcbin->Write();
  ch_adcint->Write();
  tdcbin_time->Write();
  channel_occupancy->Write();
  gainxy->Write();
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c]->Write();
  }
  asic_occupancy_all->Write();
  asic_hott->Write();
  asic_warm->Write();
  asic_cold->Write();
  digitsperchannel->Write();
  ch_time_proj->Write();
  ch_time_proj_qual->Write();
  for (int ii = 0; ii < 8; ii++) {
    nhit[ii]->Write();
    nhitq[ii]->Write();
    evtnum[ii]->Write();
    evtnumq[ii]->Write();
    pmt_xy[ii]->Write();
    pmt_xyq[ii]->Write();
    adcint[ii]->Write();
  }

  scrods_tight->Write();
  ch_widths_tight->Write();
  ch_heights_tight->Write();
  width_time_tight->Write();
  height_time_tight->Write();
  adcint_time_tight->Write();
  width_time_corr_tight->Write();
  height_time_corr_tight->Write();
  adcint_time_corr_tight->Write();
  ch_time_tight->Write();
  ch_tdcbin_tight->Write();
  ch_adcint_tight->Write();
  tdcbin_time_tight->Write();
  channel_occupancy_tight->Write();
  gainxy_tight->Write();
  for (int c = 0; c < 4; c++) {
    asic_occupancy_tight[c]->Write();
  }
  asic_occupancy_all_tight->Write();
  asic_hott_tight->Write();
  asic_warm_tight->Write();
  asic_cold_tight->Write();
  digitsperchannel_tight->Write();
  ch_time_proj_tight->Write();
  for (int ii = 0; ii < 8; ii++) {
    nhit_tight[ii]->Write();
    evtnum_tight[ii]->Write();
    pmt_xy_tight[ii]->Write();
    adcint_tight[ii]->Write();
  }

  scrods_optics->Write();
  ch_widths_optics->Write();
  ch_heights_optics->Write();
  width_time_optics->Write();
  height_time_optics->Write();
  adcint_time_optics->Write();
  width_time_corr_optics->Write();
  height_time_corr_optics->Write();
  adcint_time_corr_optics->Write();
  ch_time_optics->Write();
  ch_tdcbin_optics->Write();
  ch_adcint_optics->Write();
  tdcbin_time_optics->Write();
  channel_occupancy_optics->Write();
  gainxy_optics->Write();
  for (int c = 0; c < 4; c++) {
    asic_occupancy_optics[c]->Write();
  }
  asic_occupancy_all_optics->Write();
  asic_hott_optics->Write();
  asic_warm_optics->Write();
  asic_cold_optics->Write();
  digitsperchannel_optics->Write();
  ch_time_proj_optics->Write();
  for (int ii = 0; ii < 8; ii++) {
    nhit_optics[ii]->Write();
    evtnum_optics[ii]->Write();
    pmt_xy_optics[ii]->Write();
    adcint_optics[ii]->Write();
  }


  for (int ii = 0; ii < 64; ii++) {
    time_by_asic[ii]->Write();
    time_by_asic_tight[ii]->Write();
    time_by_asic_optics[ii]->Write();
  }

  //Expert plots from Umberto
  /*
  TH2F* ASIC_tdcbin_time[64];
  TH2F* ASIC_tdcbin_height[64];
  TH2F* ASIC_tdcbin_width[64];
  TH1F* ASIC_timing_qual[64];

  TCut tight_cut_u("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0 && TOPCAFDigits.m_adc_height > 100");
  TCut loose_cut_u("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0");
  TCut optics_cut_u("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0 && TOPCAFDigits.m_adc_height > 150 && TOPCAFDigits.m_asic_ch !=7 && TOPCAFDigits.m_tdc_bin%128 > 60");


  int itot(0);
  for (int ibs = 0; ibs < 4; ibs++) {
    for (int icr = 0; icr < 4; icr++) {
      for (int ias = 0; ias < 4; ias++) {
        ASIC_tdcbin_time[itot] = new TH2F(Form("ASIC_tdcbin_time_%d_%d_%d", ibs, icr, ias),  Form("hit time vs tdc bin in ASIC %d %d %d",ibs, icr, ias), 257, 0., 257, 2000, -1000., 1000.);
        ASIC_tdcbin_time[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_time[itot]->GetYaxis()->SetTitle("hit time [time bin]");
        t.Draw(Form("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_time_%d_%d_%d", ibs, icr, ias), loose_cut_u && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",ibs, icr, ias));
        ASIC_tdcbin_time[itot]->Write();

        ASIC_tdcbin_height[itot] = new TH2F(Form("ASIC_tdcbin_height_%d_%d_%d", ibs, icr, ias), Form("hit height vs tdc bin in ASIC %d %d %d",ibs, icr, ias), 257, 0., 257, 2000, 0., 2000);
        ASIC_tdcbin_height[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_height[itot]->GetYaxis()->SetTitle("hit height [adc bin]");
        t.Draw(Form("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_height_%d_%d_%d", ibs, icr, ias), loose_cut_u && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",ibs, icr, ias));
        ASIC_tdcbin_height[itot]->Write();

        ASIC_tdcbin_width[itot] = new TH2F(Form("ASIC_tdcbin_width_%d_%d_%d", ibs, icr, ias),  Form("hit width vs tdc bin in ASIC %d %d %d",ibs, icr, ias), 257, 0., 257, 200, 0., 20.);
        ASIC_tdcbin_width[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_width[itot]->GetYaxis()->SetTitle("hit width [adc bin]");
        t.Draw(Form("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_width_%d_%d_%d", ibs, icr, ias), loose_cut_u && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",ibs, icr, ias));
        ASIC_tdcbin_width[itot]->Write();

        ASIC_timing_qual[itot] = new TH1F(Form("ASIC_timing_qual_%d_%d_%d", ibs, icr, ias),  Form("Photon timing after quality cuts in ASIC %d %d %d",  ibs, icr, ias), 4000, -200, 200);
        ASIC_timing_qual[itot]->GetXaxis()->SetTitle("hit time [ns]");
        t.Draw(Form("TOPCAFDigits.m_time>>ASIC_timing_qual_%d_%d_%d", ibs, icr, ias), tight_cut_u && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",ibs, icr, ias));
        ASIC_timing_qual[itot]->Write();

        itot++;
      }
    }
  }
  */

  fout.Close();
  return true;

}

int main(int argc, char* argv[])
{
  if (argc == 1) {
    cout << "analyzeHits inputfile.root outputfile.root" << endl;
  } else if (argc == 2) {
    analyzeHits_extended(argv[1]);
  } else if (argc == 3) {
    analyzeHits_extended(argv[1], argv[2]);
  } else if (argc == 4) {
    analyzeHits_extended(argv[1], argv[2], argv[3]);
  } else if (argc == 5) {
    analyzeHits_extended(argv[1], argv[2], argv[3], argv[4]);
  }
  return 0;
}
