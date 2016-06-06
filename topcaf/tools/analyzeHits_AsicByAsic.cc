//
//
// Analyzer fo the TOPCAF roots
// author: Umberto Tamponi
// email: tamponi@to.infn.it
//
// May 28: adapted for the analsysi fo the installed slots (U.T.)

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

using namespace std;
using namespace Belle2;

bool analyzeHits_AsicByAsic(const char* filename, const char* outfilename = "hits.root", const char* trigpos = "")
{
  string trigstring = string(trigpos);
  cout << trigstring << endl;

//  clock_t t1 = clock();

  //detect fuji/tsukuba based on suffix
  //use cut to set trigger position

  TChain t("tree");
  t.Add(filename);
  int maxadc = 2000;


  //deifne some standard cuts to produce different plots
  TCut tight_cut("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0 && TOPCAFDigits.m_adc_height > 100 && TOPCAFDigits.m_flag == 201");
  TCut tight_cut_no7("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 8 && TOPCAFDigits.m_q > 0 && TOPCAFDigits.m_adc_height > 150 && TOPCAFDigits.m_asic_ch!=7");
  TCut loose_cut("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0");
  TCut optics_cut("TOPCAFDigits.m_width > 3 && TOPCAFDigits.m_width < 10 && TOPCAFDigits.m_q > 0 && TOPCAFDigits.m_adc_height > 150 && TOPCAFDigits.m_asic_ch !=7 && TOPCAFDigits.m_tdc_bin%128 > 60 ");


  //  TH2F* timing_difference = new TH2F("timing_difference", "channle - by channel time difference between direct and reflected peak", 512, 0., 512., 4000, -200, 200);


  //first fills the timing plots to gte the position of the primary peak
  TH1D* timing = new TH1D("timing", "time projection over all the channels", 8000, -2000, 2000.);
  timing->GetXaxis()->SetTitle("hit time [tdc bin ]");
  t.Draw("TOPCAFDigits.m_time>>timing");

  TH1D* timing_qual = new TH1D("timing_qual", "time projection over all the channels, with quality cuts", 8000, -2000, 2000.);
  timing_qual->GetXaxis()->SetTitle("hit time [tdc bin]");
  t.Draw("TOPCAFDigits.m_time>>timing_qual", tight_cut);

  TH1D* timing_optics = new TH1D("timing_optics", "time projection over all the channels, removing the electronics problems", 8000,
                                 -2000, 2000.);
  timing_qual->GetXaxis()->SetTitle("hit time [tdc bin]");
  t.Draw("TOPCAFDigits.m_time>>timing_optics", optics_cut);

  TH1D* timing_max = new TH1D("timing_max", "time projection over all the channels, with quality cuts", 400, -2000., 0.);
  t.Draw("TOPCAFDigits.m_time>>timing_max", tight_cut_no7);

  double max = timing_max->GetBinCenter(timing_max->GetMaximumBin());




  TCut laser_dir(Form("TMath::Abs(TOPCAFDigits.m_time - %f) < 12 ", max));
  TCut laser_refl(Form("TMath::Abs(TOPCAFDigits.m_time  - ( %f + 80)) < 20 ", max));
  TCut cosmic_dir(Form("TMath::Abs(TOPCAFDigits.m_time -  %f) < 18 ", max));
  TCut cosmic_refl(Form("TMath::Abs(TOPCAFDigits.m_time - ( %f + 70)) < 40 ", max));
  TCut mirror_dir(Form("TMath::Abs(TOPCAFDigits.m_time -  %f) < 10 ", max));
  TCut mirror_refl(Form("TMath::Abs(TOPCAFDigits.m_time - ( %f + 30)) < 10 ", max));


  TH2F* ch_time_qual_ROI_laser = new TH2F("ch_time_qual_ROI_laser",
                                          "time projection over all the channels, with quality cuts. Only ROI used for occupancy maps (this is cross  check plot!)", 512, 0,
                                          512, 8000, -2000, 2000.);
  ch_time_qual_ROI_laser->GetYaxis()->SetTitle("hit time [ns]");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual_ROI_laser", tight_cut && (laser_dir || laser_refl));

  TH2F* ch_time_qual_ROI_midbar = new TH2F("ch_time_qual_ROI_midbar",
                                           "time projection over all the channels, with quality cuts. Only ROI used for occupancy maps (this is cross  check plot!)", 512, 0,
                                           512, 8000, -2000, 2000.);
  ch_time_qual_ROI_midbar->GetYaxis()->SetTitle("hit time [ns]");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual_ROI_midbar", tight_cut && (cosmic_dir || cosmic_refl));

  TH2F* ch_time_qual_ROI_mirror = new TH2F("ch_time_qual_ROI_mirror",
                                           "time projection over all the channels, with quality cuts. Only ROI used for occupancy maps (this is cross  check plot!)", 512, 0,
                                           512, 8000, -2000, 2000.);
  ch_time_qual_ROI_mirror->GetYaxis()->SetTitle("hit time [ns]");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual_ROI_mirror", tight_cut && (mirror_dir || mirror_refl));



  //Define the cuts here using a TSpectrum to search first


  TFile fout(outfilename, "RECREATE");

  // declares some basic histograms without any cut applied
  TH1F* scrods = new TH1F("scrods", "scrod id numbers (no cuts)", 100, 0., 100.);
  scrods->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths = new TH2F("ch_widths", "hit width vs pixel", 512, 0., 512., 1000, 0., 100.);
  ch_widths->GetXaxis()->SetTitle("pixel");
  ch_widths->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights = new TH2F("ch_heights", "hit height vs pixel", 512, 0., 512., 200, -2048., 2048.);
  ch_heights->GetXaxis()->SetTitle("pixel");
  ch_heights->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* ch_adcint = new TH2F("ch_adcint", "adcint vs pixel", 512, 0., 512., 500, 0, 10000);
  ch_adcint->GetXaxis()->SetTitle("pixel");
  ch_adcint->GetYaxis()->SetTitle("adc int");

  TH2F* ch_time = new TH2F("ch_time", "time vs pixel", 512, 0., 512., 40000, -2000., 2000.);
  ch_time->GetXaxis()->SetTitle("pixel");
  ch_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* width_time = new TH2F("width_time", "widths vs times", 2500, 0., 2500., 1000, 0., 100.);
  width_time->GetXaxis()->SetTitle("hit tdc bin");
  width_time->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time = new TH2F("height_time", "heights vs times (quality cuts)", 2500, 0., 2500., 1000, -2000., 2000.);
  height_time->GetXaxis()->SetTitle("hit tdc bin");
  height_time->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time = new TH2F("adcint_time", "adcint vs times", 2500, 0., 2500., 500, 0, 10000);
  adcint_time->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin->GetXaxis()->SetTitle("pixel");
  ch_tdcbin->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time = new TH2F("tdcbin_time", "hit time vs tdc bin", 1500, 0., (double)maxadc, 4000, -2000., 2000.);
  tdcbin_time->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* hitmap = new TH2F("hitmap", "pmt hits vs pmt xy position", 64, 0., 64., 8, 0., 8.);
  hitmap->GetXaxis()->SetTitle("pmt x");
  hitmap->GetYaxis()->SetTitle("pmt y");
  hitmap->SetStats(0);

  TH2F* pulser_height_width = new TH2F("pulser_height_width", "pulser signal: height VS width", 200, 0., 20., 1000, -1000., 0.);
  pulser_height_width->GetXaxis()->SetTitle(" width [tdc bin]");
  pulser_height_width->GetYaxis()->SetTitle(" amplitude [acd counts]");


  // declares some basic histograms without any cut applied
  TH1F* scrods_qual = new TH1F("scrods_qual", "scrod id numbers (no cuts)", 100, 0., 100.);
  scrods_qual->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths_qual = new TH2F("ch_widths_qual", "hit width vs pixel", 512, 0., 512., 1000, 0., 100.);
  ch_widths_qual->GetXaxis()->SetTitle("pixel");
  ch_widths_qual->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights_qual = new TH2F("ch_heights_qual", "hit height vs pixel", 512, 0., 512., 200, -2048., 2048.);
  ch_heights_qual->GetXaxis()->SetTitle("pixel");
  ch_heights_qual->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* ch_adcint_qual = new TH2F("ch_adcint_qual", "adcint vs pixel", 512, 0., 512., 500, 0, 10000);
  ch_adcint_qual->GetXaxis()->SetTitle("pixel");
  ch_adcint_qual->GetYaxis()->SetTitle("adc int");

  TH2F* ch_time_qual = new TH2F("ch_time_qual", "time vs pixel", 512, 0., 512., 40000, -2000., 2000.);
  ch_time_qual->GetXaxis()->SetTitle("pixel");
  ch_time_qual->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* width_time_qual = new TH2F("width_time_qual", "widths vs times", 2500, 0., 2500., 1000, 0., 100.);
  width_time_qual->GetXaxis()->SetTitle("hit tdc bin");
  width_time_qual->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time_qual = new TH2F("height_time_qual", "heights vs times (quality cuts)", 2500, 0., 2500., 1000, -2000., 2000.);
  height_time_qual->GetXaxis()->SetTitle("hit tdc bin");
  height_time_qual->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* adcint_time_qual = new TH2F("adcint_time_qual", "adcint vs times", 2500, 0., 2500., 500, 0, 10000);
  adcint_time_qual->GetXaxis()->SetTitle("hit tdc bin");
  adcint_time_qual->GetYaxis()->SetTitle("adcint [adc bin]");

  TH2F* ch_tdcbin_qual = new TH2F("ch_tdcbin_qual", "tdc bin vs channel", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin_qual->GetXaxis()->SetTitle("pixel");
  ch_tdcbin_qual->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time_qual = new TH2F("tdcbin_time_qual", "hit time vs tdc bin", 1500, 0., (double)maxadc, 4000, -2000., 2000.);
  tdcbin_time_qual->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time_qual->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* hitmap_qual = new TH2F("hitmap_qual", "pmt hits vs pmt xy position", 64, 0., 64., 8, 0., 8.);
  hitmap_qual->GetXaxis()->SetTitle("pmt x");
  hitmap_qual->GetYaxis()->SetTitle("pmt y");
  hitmap_qual->SetStats(0);


  TH1F* asic_occupancy[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c] = new TH1F(Form("asic_occupancy%d", c),
                                 Form("boardstack %d laser hits in each asic (qual cuts and primary laser times)", c), 350, 0., 350.);
    asic_occupancy[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy[c]->GetYaxis()->SetTitle("counts");
  }




  TH2F* pmtxy_laserDirect =  new TH2F("pmtxy_laserDirect", "hitmap of the direct laser light", 64, 0., 64., 8, 0., 8.);
  TH2F* pmtxy_laserReflected = new TH2F("pmtxy_laserReflected", "hitmap of the reflected laser light", 64, 0., 64., 8, 0., 8.);

  TH2F* pmtxy_midbarDirect = new TH2F("pmtxy_midbarDirect", "hitmap of the direct cosmic light with paddles at z =0", 64, 0., 64., 8,
                                      0., 8.);
  TH2F* pmtxy_midbarReflected = new TH2F("pmtxy_midbarReflected", "hitmap of the reflected cosmic light with paddles at z =0", 64, 0.,
                                         64., 8, 0., 8.);

  //  TH2F* pmtxy_mirrorDirect = new TH2F("pmtxy_mirrorDirect", "hitmap of the direct cosmic light with paddles at z = mirror", 64, 0.,
  //                                    64., 8, 0., 8.);
//  TH2F* pmtxy_mirrorReflected = new TH2F("pmtxy_mirrorReflected", "hitmap of the reflected cosmic light with paddles at z = mirror",
  //                                       64, 0., 64., 8, 0., 8.);

  TH1F* evtnum = new TH1F("evtnum", "event num", 1000, 0, 16000);
  TH1F* nhit = new TH1F("nhit", "hit num", 100, 0, 100);
  TH1F* evtnum_qual = new TH1F("evtnum_qual", "even num qual", 1000, 0, 16000);
  TH1F* nhit_qual = new TH1F("nhit_qual", "hit num qual", 100, 0, 100);

  TH2F* ASIC_tdcbin_time[64];
  TH2F* ASIC_tdcbin_height[64];
  TH2F* ASIC_tdcbin_width[64];
  TH1F* ASIC_timing_qual[64];


  TH1F* timing_qual_ch[8];



  fout.cd();



  t.Draw("(TOPCAFDigits.m_evt_num)>>evtnum", "TOPCAFDigits.m_adc_height > 0");
  t.Draw("(TOPCAFDigits.m_evt_num)>>evtnum_qual", tight_cut);


  for (int i = 0; i < 1000; i++) {
    nhit->Fill(evtnum->GetBinContent(i));
    nhit_qual->Fill(evtnum_qual->GetBinContent(i));
  }

  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_width>>pulser_height_width",
         "TOPCAFDigits.m_adc_height< 0 && TOPCAFDigits.m_asic_ch==7");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", tight_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_laserDirect",  laser_dir && tight_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_laserReflected",  laser_refl && tight_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_midbarDirect",  cosmic_dir && tight_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_midbarReflected",  cosmic_refl && tight_cut);
  // t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_mirrorDirect",  mirror_dir && tight_cut);
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_mirrorReflected",  mirror_refl && tight_cut);


  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy%d", c), tight_cut
           && Form("TOPCAFDigits.m_boardstack==%d", c));
  }


  t.Draw("TOPCAFDigits.m_scrod_id>>scrods_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_pixel_id>>ch_adcint_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_tdc_bin>>adcint_time_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time_qual", tight_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", tight_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>hitmap_qual", tight_cut);

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods");
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths");
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights");
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_pixel_id>>ch_adcint");
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time");
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time");
  t.Draw("TOPCAFDigits.m_q:TOPCAFDigits.m_tdc_bin>>adcint_time");
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time");
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time");
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>hitmap");



  pmtxy_laserDirect->Write();
  pmtxy_laserReflected->Write();
  pmtxy_midbarDirect->Write();
  pmtxy_midbarReflected->Write();
  //  pmtxy_mirrorDirect->Write();
  //  pmtxy_mirrorReflected->Write();
  ch_time_qual->Write();
  timing_qual->Write();
  timing_optics->Write();
  pulser_height_width->Write();
  evtnum_qual->Write();
  nhit_qual->Write();
  scrods_qual->Write();
  ch_widths_qual->Write();
  ch_heights_qual->Write();
  width_time_qual->Write();
  height_time_qual->Write();
  adcint_time_qual->Write();
  ch_tdcbin_qual->Write();
  ch_adcint_qual->Write();
  tdcbin_time_qual->Write();
  hitmap_qual->Write();


  for (int c = 0; c < 4; c++) {
    asic_occupancy[c]->Write();
  }


  for (int ich = 0; ich < 8; ich++) {
    timing_qual_ch[ich] = new TH1F(Form("time_qual_ch_%d", ich), Form("timing on channel %d across all the asics", ich), 8000, -2000.,
                                   2000.);
    timing_qual_ch[ich]->GetYaxis()->SetTitle("hit time [time bin]");
    t.Draw(Form("TOPCAFDigits.m_time>>time_qual_ch_%d", ich), tight_cut && Form("TOPCAFDigits.m_asic_ch == %d", ich));
    timing_qual_ch[ich]->Write();
  }


  evtnum->Write();
  nhit->Write();
  scrods->Write();
  ch_widths->Write();
  ch_heights->Write();
  width_time->Write();
  height_time->Write();
  adcint_time->Write();
  timing->Write();
  ch_tdcbin->Write();
  ch_adcint->Write();
  ch_time->Write();
  tdcbin_time->Write();
  hitmap->Write();

  ch_time_qual_ROI_laser->Write();
  ch_time_qual_ROI_midbar->Write();
  //  ch_time_qual_ROI_mirror->Write();


  int itot = 0;

  for (int ibs = 0; ibs < 4; ibs++) {
    for (int icr = 0; icr < 4; icr++) {
      for (int ias = 0; ias < 4; ias++) {
        ASIC_tdcbin_time[itot] = new TH2F(Form("ASIC_tdcbin_time_%d_%d_%d", ibs, icr, ias),  Form("hit time vs tdc bin in ASIC %d %d %d",
                                          ibs, icr, ias), 257, 0., 257, 3000, -2000., 1000.);
        ASIC_tdcbin_time[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_time[itot]->GetYaxis()->SetTitle("hit time [time bin]");
        t.Draw(Form("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_time_%d_%d_%d", ibs, icr, ias), loose_cut
               && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",   ibs, icr, ias));
        ASIC_tdcbin_time[itot]->Write();

        ASIC_tdcbin_height[itot] = new TH2F(Form("ASIC_tdcbin_height_%d_%d_%d", ibs, icr, ias),
                                            Form("hit height vs tdc bin in ASIC %d %d %d",  ibs, icr, ias), 257, 0., 257, 2000, 0., 2000);
        ASIC_tdcbin_height[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_height[itot]->GetYaxis()->SetTitle("hit height [adc bin]");
        t.Draw(Form("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_height_%d_%d_%d", ibs, icr, ias), loose_cut
               && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",   ibs, icr, ias));
        ASIC_tdcbin_height[itot]->Write();

        ASIC_tdcbin_width[itot] = new TH2F(Form("ASIC_tdcbin_width_%d_%d_%d", ibs, icr, ias),  Form("hit width vs tdc bin in ASIC %d %d %d",
                                           ibs, icr, ias), 257, 0., 257, 200, 0., 20.);
        ASIC_tdcbin_width[itot]->GetXaxis()->SetTitle("hit time bin %256 [time bin]");
        ASIC_tdcbin_width[itot]->GetYaxis()->SetTitle("hit width [adc bin]");
        t.Draw(Form("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin%%256>>ASIC_tdcbin_width_%d_%d_%d", ibs, icr, ias), loose_cut
               && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",   ibs, icr, ias));
        ASIC_tdcbin_width[itot]->Write();

        ASIC_timing_qual[itot] = new TH1F(Form("ASIC_timing_qual_%d_%d_%d", ibs, icr, ias),
                                          Form("Photon timing after quality cuts_%d_%d_%d",
                                               ibs, icr, ias), 8000, -2000, 2000);

//        ASIC_timing_qual[itot] = new TH1F(Form("ASIC_timing_qual_%d_%d_%d", ibs, icr, ias),  Form("Photon timing after quality cuts",  ibs,
//                                          icr, ias), 4000, -200, 200); //wxl, warning: too many arguments for format

        ASIC_timing_qual[itot]->GetXaxis()->SetTitle("hit time [ns]");
        t.Draw(Form("TOPCAFDigits.m_time>>ASIC_timing_qual_%d_%d_%d", ibs, icr, ias), tight_cut
               && Form("TOPCAFDigits.m_boardstack == %d && TOPCAFDigits.m_asic_row == %d && TOPCAFDigits.m_asic == %d",   ibs, icr, ias));
        ASIC_timing_qual[itot]->Write();

        itot++;
      }
    }
  }


  fout.Close();
  return true;

}

int main(int argc, char* argv[])
{
  if (argc == 1) {
    cout << "analyzeHits inputfile.root outputfile.root" << endl;
  } else if (argc == 2) {
    analyzeHits_AsicByAsic(argv[1]);
  } else if (argc == 3) {
    analyzeHits_AsicByAsic(argv[1], argv[2]);
  } else if (argc == 4) {
    analyzeHits_AsicByAsic(argv[1], argv[2], argv[3]);
  }
  return 0;
}
