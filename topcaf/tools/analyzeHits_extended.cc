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

bool analyzeHits_extended(const char* filename, const char* outfilename = "hits.root", const char* trigpos = "")
{
  string trigstring = string(trigpos);
  cout << trigstring << endl;

  clock_t t1 = clock();

  //detect fuji/tsukuba based on suffix
  //use cut to set trigger position

  TChain t("tree");
  t.Add(filename);
  int maxadc = 2000;

  TFile fout(outfilename, "RECREATE");

  TH1F* scrods = new TH1F("scrods", "scrod id numbers (no cuts)", 100, 0., 100.);
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

  TH2F* ch_time_qual = new TH2F("ch_time_qual", "time vs pixel (quality cuts)", 512, 0., 512., 40000, -2000., 2000.);
  ch_time->GetXaxis()->SetTitle("pixel");
  ch_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH1D* ch_time_proj_qual = new TH1D("ch_time_proj_qual", "time projection (qual cuts)", 40000, -2000., 2000.);
  ch_time_proj_qual->GetXaxis()->SetTitle("time");

  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel (quality cuts)", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin->GetXaxis()->SetTitle("pixel");
  ch_tdcbin->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time = new TH2F("tdcbin_time", "hit time vs tdc bin (quality cuts)", 1500, 0., (double)maxadc, 4000, -2000., 2000.);
  tdcbin_time->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* pmtxy = new TH2F("pmtxy", "pmt hits vs pmt xy position (good events)", 64, 0., 64., 8, 0., 8.);
  pmtxy->GetXaxis()->SetTitle("pmt x");
  pmtxy->GetYaxis()->SetTitle("pmt y");
  pmtxy->SetStats(0);

  TH2F* gainxy = new TH2F("gainxy", "adcint vs pmt xy position (good events)", 64, 0., 64., 8, 0., 8.);
  gainxy->GetXaxis()->SetTitle("pmt x");
  gainxy->GetYaxis()->SetTitle("pmt y");
  gainxy->SetStats(0);


  TH1F* laser_occupancy = new TH1F("laser_occupancy", "laser hits in each pixel (qual cuts and primary laser times)", 512, 0., 512.);
  laser_occupancy->GetXaxis()->SetTitle("pixel");
  laser_occupancy->GetYaxis()->SetTitle("counts");

  TH1F* digitsperchannel = new TH1F("digitsperchannel", "TOPCAF digits per channel", 8, 0, 8);
  digitsperchannel->GetXaxis()->SetTitle("channel");


  TH1F* asic_occupancy[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c] = new TH1F(Form("asic_occupancy%d", c),
                                 Form("boardstack %d laser hits in each asic (qual cuts and primary laser times)", c), 350, 0., 350.);
    asic_occupancy[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy[c]->GetYaxis()->SetTitle("counts");
  }

  TH1F* evtnum[10];
  TH1F* evtnumq[10];
  TH1F* nhit[10];
  TH1F* nhitq[10];
  TCut cosmic_cut[10];
  TH2F* pmt_xy[10];
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
    sprintf(n_adcint, "adcint%d", ii);
    adcint[ii] = new TH1F(n_adcint, n_adcint, 1000, 0, 10000);
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

  const int flag_lab = strstr(filename, "cpr3015") != NULL;
  //0=fuji, 1=tsukuba
  cout << flag_lab << endl;
  const int flag_m0809 = (strstr(filename, "module08") != NULL) || (strstr(filename, "module09") != NULL);
  //0=false, 1=true
  cout << flag_m0809 << endl;
  const int flag_m02 = (strstr(filename, "module02") != NULL);
  //0=false, 1=true
  cout << flag_m02 << endl;

  if (flag_lab == 0) {
    //FUJI
    if (trigstring == "mirror") {
      if (flag_m0809 == 1) {
        cout << "mirror0809" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-1217&&TOPCAFDigits.m_time<-1191"); //-1204 m09
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-1133&&TOPCAFDigits.m_time<-1101"); //-1117 m09
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-1250&&TOPCAFDigits.m_time<-1226"); //-1238
        cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
      } else {
        cout << "mirror" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-1206&&TOPCAFDigits.m_time<-1180"); //-1191 (-1193 m05, -1188 m02)
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-1123&&TOPCAFDigits.m_time<-1091"); //-1112 (-1107 m05, -1101 m02)
        if (flag_m02 == 1) {
          cosmic_cut[0] = ("TOPCAFDigits.m_time>-1201&&TOPCAFDigits.m_time<-1175"); //-1191 (-1193 m05, -1188 m02)
          cosmic_cut[1] = ("TOPCAFDigits.m_time>-1117&&TOPCAFDigits.m_time<-1085"); //-1112 (-1107 m05, -1101 m02)
        }
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-1265&&TOPCAFDigits.m_time<-1220"); //-1230
        cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
      }
    }
    if (trigstring == "midbar") {
      cout << "midbar" << endl;
      cosmic_cut[0] = ("TOPCAFDigits.m_time>-1243&&TOPCAFDigits.m_time<-1211"); //-1226
      cosmic_cut[1] = ("TOPCAFDigits.m_time>-1169&&TOPCAFDigits.m_time<-1145"); //-1156
      cosmic_cut[2] = ("TOPCAFDigits.m_time>-1076&&TOPCAFDigits.m_time<-1076"); //-1076
      cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
      cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
      cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
    }
    if (trigstring == "prism") {
      if (flag_m0809 == 1) {
        cout << "prism0809" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-1236&&TOPCAFDigits.m_time<-1186"); //-1245
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-1144&&TOPCAFDigits.m_time<-1104"); //-1155
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-1265&&TOPCAFDigits.m_time<-1220"); //-1070
        cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
      } else {
        cout << "prism" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-1226&&TOPCAFDigits.m_time<-1176"); //-1235
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-1137&&TOPCAFDigits.m_time<-1097"); //-1148
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-1265&&TOPCAFDigits.m_time<-1220"); //-1065
        cosmic_cut[3] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315");
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-1050");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>-650&&TOPCAFDigits.m_time<-400");
      }
    }
  } else {
    //TSUKUBA
    if (trigstring == "laser") {
      cout << "laser" << endl;
      cosmic_cut[0] = ("TOPCAFDigits.m_time>30&&TOPCAFDigits.m_time<50");
      cosmic_cut[1] = ("TOPCAFDigits.m_time>115&&TOPCAFDigits.m_time<130");
      cosmic_cut[2] = ("TOPCAFDigits.m_time>60&&TOPCAFDigits.m_time<80");
      cosmic_cut[3] = ("TOPCAFDigits.m_time>30&&TOPCAFDigits.m_time<50");
      cosmic_cut[4] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<10");
      cosmic_cut[5] = ("TOPCAFDigits.m_time>85&&TOPCAFDigits.m_time<110");
    }
    if (trigstring == "mirror") {
      if (flag_m0809 == 1) {
        cout << "mirror0809" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-427&&TOPCAFDigits.m_time<-401");
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-345&&TOPCAFDigits.m_time<-313");
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-471&&TOPCAFDigits.m_time<-447"); //-459
        cosmic_cut[3] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<1"); //laser
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-550&&TOPCAFDigits.m_time<-300");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<10");
      } else {
        cout << "mirror" << endl;
        cosmic_cut[0] = ("TOPCAFDigits.m_time>-279&&TOPCAFDigits.m_time<-249");
        cosmic_cut[1] = ("TOPCAFDigits.m_time>-209&&TOPCAFDigits.m_time<-159");
        cosmic_cut[2] = ("TOPCAFDigits.m_time>-320&&TOPCAFDigits.m_time<-296"); //-308
        cosmic_cut[3] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<1"); //laser
        cosmic_cut[4] = ("TOPCAFDigits.m_time>-400&&TOPCAFDigits.m_time<-150");
        cosmic_cut[5] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<10");
      }
    }
  }

  TCut width_cut("TOPCAFDigits.m_width>3.&&TOPCAFDigits.m_width<20.");
  TCut height_cut("TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<2048 && TOPCAFDigits.m_adc_height<900");
  TCut height_cut2("TOPCAFDigits.m_adc_height>200&&TOPCAFDigits.m_adc_height<2048 && TOPCAFDigits.m_adc_height<900");
  TCut qual_cut("TOPCAFDigits.m_flag>0&&TOPCAFDigits.m_corr_time!=0");
  //TCut qual_cut("TOPCAFDigits.m_flag>99 && TOPCAFDigits.m_flag<999 &&TOPCAFDigits.m_corr_time!=0"); //clean up the cal pulse?
  //TCut pmt_quality_cut("int((TOPCAFDigits.m_pixel_id-1)/64)==1 && ((TOPCAFDigits.m_pixel_id-1)%64>=16 && (TOPCAFDigits.m_pixel_id-1)%64<24)");
  //TCut pmt_quality_cut("(TOPCAFDigits.m_pixel_id>160 && TOPCAFDigits.m_pixel_id<169) || (TOPCAFDigits.m_pixel_id>64 && TOPCAFDigits.m_pixel_id<73) || (TOPCAFDigits.m_pixel_id>480 && TOPCAFDigits.m_pixel_id<489)");
  TCut pmt_quality_cut("(TOPCAFDigits.m_pixel_id<-100)");

  t2 = clock();
  cout << "ready " << (t2 - t1) << endl;
  t1 = t2;

  for (int ii = 0; ii < 8; ii++) {
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnum%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnumq%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut2);
    TString ts1 = "(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>";
    TString ts2 = Form("pmt_xy%d", ii);
    ts1 = ts1 + ts2;
    //t.Draw(Form("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmt_xy%d",ii),width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    t.Draw(ts1, width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    pmt_xy[ii]->SetOption("COLZ");
    pmt_xy[ii]->Draw();
    t.Draw(Form("(TOPCAFDigits.m_q)>>adcint%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    for (int i = 0; i < 1000; i++) {
      nhit[ii]->Fill(evtnum[ii]->GetBinContent(i));
      nhitq[ii]->Fill(evtnumq[ii]->GetBinContent(i));
    }
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
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", width_cut && qual_cut && height_cut2 && !pmt_quality_cut);
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy", width_cut && qual_cut && cosmic_primary_cut && height_cut2 && !pmt_quality_cut, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all", width_cut && qual_cut && cosmic_primary_cut && height_cut && !pmt_quality_cut, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy", width_cut && qual_cut && cosmic_cut[2] && height_cut2, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all", width_cut && qual_cut && cosmic_cut[2] && height_cut && !pmt_quality_cut, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_prism", width_cut && qual_cut && cosmic_cut[4] && height_cut2, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all_prism", width_cut && qual_cut && cosmic_cut[4] && height_cut, "COLZ");
  t.Draw("TOPCAFDigits.m_pixel_id>>laser_occupancy", width_cut && qual_cut && cosmic_cut[0] && height_cut);

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
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy%d", c), width_cut
           && qual_cut && Form("TOPCAFDigits.m_boardstack==%d", c));
  }
  t.Draw("TOPCAFDigits.m_asic_ch>>digitsperchannel", height_cut);

  ch_time_proj = ch_time->ProjectionY();
  ch_time_proj->Rebin(10);
  ch_time_proj_qual = ch_time_qual->ProjectionY();
  ch_time_proj_qual->Rebin(10);

  /*
  const int npeaks=3;
  TH1F *roi = (TH1F*)ch_time_proj_qual->Clone();
  roi->Rebin(2);
  roi->GetXaxis()->SetRangeUser(-1300,-1050);
  TSpectrum *s = new TSpectrum(npeaks);
  int nfound = s->Search(roi,1,"",0.01);
  Double_t *xpeaks=s->GetPositionX();

  TF1 **fn = new TF1*[npeaks];
  for(int ii=0;ii<npeaks;ii++){
    char fname[20];
    sprintf(fname,"f%d",ii);
    fn[ii]=new TF1(fname,"gaus");
  }

  for(int p=0;p<nfound;p++){
    Float_t xp = xpeaks[p];
    fn[p]->SetParameters(3000, xp, 5);
    Float_t max_y = 1000000;
    fn[p]->SetParLimits(0,100,max_y);
    fn[p]->SetParLimits(1,xp-2,xp+2);
    fn[p]->SetParLimits(2,1,30);
    fn[p]->SetLineColor(kGreen);
    ch_time_proj_qual->Fit(fn[p], "QNM", "", xp-10, xp+10);
    cout<<fn[p]->GetParameter(1)<<" "<<fn[p]->GetParameter(2)<<endl;
  }
  */

  /*
  pmtxy->SetOption("COLZ");
  pmtxy->Draw();
  pmtxy_all->SetOption("COLZ");
  pmtxy_all->Draw();
  pmtxy_prism->SetOption("COLZ");
  pmtxy_prism->Draw();
  pmtxy_all_prism->SetOption("COLZ");
  pmtxy_all_prism->Draw();
  */

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
  laser_occupancy->Write();
  gainxy->Write();
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c]->Write();
  }
  digitsperchannel->Write();
  ch_time_proj->Write();
  ch_time_proj_qual->Write();
  for (int ii = 0; ii < 8; ii++) {
    nhit[ii]->Write();
    nhitq[ii]->Write();
    evtnum[ii]->Write();
    evtnumq[ii]->Write();
    pmt_xy[ii]->Write();
    adcint[ii]->Write();
  }


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
  }
  return 0;
}
