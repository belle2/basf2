#include <TFile.h>
#include <TTree.h>
#include <TCut.h>
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

using namespace std;
using namespace Belle2;

bool analyzeHits(const char* filename, const char* outfilename = "hits.root")
{

  TChain t("tree");
  t.Add(filename);
  int maxadc = 2000;

  TFile fout(outfilename, "RECREATE");
  //TFile fout("hits.root", "RECREATE");

  TH1F* scrods = new TH1F("scrods", "scrod id numbers (no cuts)", 100, 0., 100.);
  scrods->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths = new TH2F("ch_widths", "hit width vs pixel (quality cuts)", 512, 0., 512., 1000, 0., 100.);
  ch_widths->GetXaxis()->SetTitle("pixel");
  ch_widths->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights = new TH2F("ch_heights", "hit height vs pixel (quality cuts)", 512, 0., 512., 200, -2048., 2048.);
  ch_heights->GetXaxis()->SetTitle("pixel");
  ch_heights->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* width_time = new TH2F("width_time", "widths vs times (quality cuts)", 2500, 0., 2500., 1000, 0., 100.);
  width_time->GetXaxis()->SetTitle("hit tdc bin");
  width_time->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* height_time = new TH2F("height_time", "heights vs times (quality cuts)", 2500, 0., 2500., 1000, 0., 2000.);
  height_time->GetXaxis()->SetTitle("hit tdc bin");
  height_time->GetYaxis()->SetTitle("hit height [adc bin]");

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

  TH2F* tdcbin_time = new TH2F("tdcbin_time", "hit time vs tdc bin (quality cuts)", 1500, 0., (double)maxadc, 30000, -2000., 2000.);
  tdcbin_time->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* pmtxy = new TH2F("pmtxy", "pmt hits vs pmt xy position (good events)", 64, 0., 64., 8, 0., 8.);
  pmtxy->GetXaxis()->SetTitle("pmt x");
  pmtxy->GetYaxis()->SetTitle("pmt y");
  pmtxy->SetStats(0);

  TH2F* pmtxy_all = new TH2F("pmtxy_all", "pmt hits vs pmt xy position (all)", 64, 0., 64., 8, 0., 8.);
  pmtxy_all->GetXaxis()->SetTitle("pmt x");
  pmtxy_all->GetYaxis()->SetTitle("pmt y");
  pmtxy_all->SetStats(0);

  TH2F* pmtxy_prism = new TH2F("pmtxy_prism", "pmt hits vs pmt xy position (good events, prism)", 64, 0., 64., 8, 0., 8.);
  pmtxy_prism->GetXaxis()->SetTitle("pmt x");
  pmtxy_prism->GetYaxis()->SetTitle("pmt y");
  pmtxy_prism->SetStats(0);

  TH2F* pmtxy_all_prism = new TH2F("pmtxy_all_prism", "pmt hits vs pmt xy position (all, prism)", 64, 0., 64., 8, 0., 8.);
  pmtxy_all_prism->GetXaxis()->SetTitle("pmt x");
  pmtxy_all_prism->GetYaxis()->SetTitle("pmt y");
  pmtxy_all_prism->SetStats(0);

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
  for (int ii = 0; ii < 10; ii++) {
    char n_evt[20], n_hit[20];
    sprintf(n_evt, "evtnum%d", ii);
    sprintf(n_hit, "nhit%d", ii);
    evtnum[ii] = new TH1F(n_evt, n_evt, 1000, 0, 16000);
    nhit[ii] = new TH1F(n_hit, n_hit, 100, 0, 100);
    sprintf(n_evt, "evtnumq%d", ii);
    sprintf(n_hit, "nhitq%d", ii);
    evtnumq[ii] = new TH1F(n_evt, n_evt, 1000, 0, 16000);
    nhitq[ii] = new TH1F(n_hit, n_hit, 100, 0, 100);
  }
  cosmic_cut[0] = ("TOPCAFDigits.m_time>-1500&&TOPCAFDigits.m_time<-200");
  //m0809 values
  cosmic_cut[1] = ("TOPCAFDigits.m_time>-1265&&TOPCAFDigits.m_time<-1220");
  //cosmic_cut[2]=("TOPCAFDigits.m_time>-1215&&TOPCAFDigits.m_time<-1190");
  //cosmic_cut[3]=("TOPCAFDigits.m_time>-1135&&TOPCAFDigits.m_time<-1105");
  cosmic_cut[2] = ("TOPCAFDigits.m_time>-1226&&TOPCAFDigits.m_time<-1176"); //fuji mo613 mirror
  cosmic_cut[3] = ("TOPCAFDigits.m_time>-1137&&TOPCAFDigits.m_time<-1097"); //fuji m0613 mirror
  //cosmic_cut[2]=("TOPCAFDigits.m_time>-1243&&TOPCAFDigits.m_time<-1211"); //fuji middle
  //cosmic_cut[3]=("TOPCAFDigits.m_time>-1169&&TOPCAFDigits.m_time<-1145"); //fuji middle
  cosmic_cut[2] = ("TOPCAFDigits.m_time>-279&&TOPCAFDigits.m_time<-249"); //tsukuba cosmic m04
  cosmic_cut[3] = ("TOPCAFDigits.m_time>-209&&TOPCAFDigits.m_time<-159"); //tsukuba cosmic m04
  cosmic_cut[2] = ("TOPCAFDigits.m_time>-427&&TOPCAFDigits.m_time<-401"); //tsukuba cosmic m09
  cosmic_cut[3] = ("TOPCAFDigits.m_time>-345&&TOPCAFDigits.m_time<-313"); //tsukuba cosmic m09
  //cosmic_cut[2]=("TOPCAFDigits.m_time>-1216&&TOPCAFDigits.m_time<-1190");  //fuji cosmic m09mirror
  //cosmic_cut[3]=("TOPCAFDigits.m_time>-1135&&TOPCAFDigits.m_time<-1103");  //fuji cosmic m09 mirror

  //cosmic_cut[2]=("TOPCAFDigits.m_time>30&&TOPCAFDigits.m_time<50"); //tsukuba laser
  //cosmic_cut[2]=("TOPCAFDigits.m_time>115&&TOPCAFDigits.m_time<130"); //tsukuba laser
  //cosmic_cut[2]=("TOPCAFDigits.m_time>60&&TOPCAFDigits.m_time<80"); //tsukuba laser
  //cosmic_cut[1]=("TOPCAFDigits.m_time>-1265&&TOPCAFDigits.m_time<-1220");
  //cosmic_cut[2]=("TOPCAFDigits.m_time>-1217&&TOPCAFDigits.m_time<-1167");
  //cosmic_cut[3]=("TOPCAFDigits.m_time>-1127&&TOPCAFDigits.m_time<-1087");
  //m0809 values
  //cosmic_cut[4]=("TOPCAFDigits.m_time>-1260&&TOPCAFDigits.m_time<-1235");
  //cosmic_cut[5]=("TOPCAFDigits.m_time>-1175&&TOPCAFDigits.m_time<-1145");
  //cosmic_cut[4] = ("TOPCAFDigits.m_time>-1262&&TOPCAFDigits.m_time<-1232");
  cosmic_cut[5] = ("TOPCAFDigits.m_time>-1172&&TOPCAFDigits.m_time<-1142");
// cosmic_cut[4] = ("TOPCAFDigits.m_time>0&&TOPCAFDigits.m_time<10"); //tsukuba laser
  cosmic_cut[4] = ("TOPCAFDigits.m_time>-340&&TOPCAFDigits.m_time<-315"); //fuji laser
  //m0613 values
  //cosmic_cut[4]=("TOPCAFDigits.m_time>-1250&&TOPCAFDigits.m_time<-1220");
  //cosmic_cut[5]=("TOPCAFDigits.m_time>-1163&&TOPCAFDigits.m_time<-1133");
  cosmic_cut[6] = ("TOPCAFDigits.m_time>-1100&&TOPCAFDigits.m_time<-1050");
  cosmic_cut[7] = ("TOPCAFDigits.m_time>-700&&TOPCAFDigits.m_time<-200");



  TCut width_cut("TOPCAFDigits.m_width>2.&&TOPCAFDigits.m_width<20.");
  TCut height_cut("TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<2048");
  TCut height_cut2("TOPCAFDigits.m_adc_height>200&&TOPCAFDigits.m_adc_height<2048");
  TCut qual_cut("TOPCAFDigits.m_flag>0&&TOPCAFDigits.m_corr_time!=0");
  //TCut qual_cut("TOPCAFDigits.m_flag>99 && TOPCAFDigits.m_flag<999 &&TOPCAFDigits.m_corr_time!=0");
  //TCut laser_primary_cut("TOPCAFDigits.m_time>190&&TOPCAFDigits.m_time<210");
  TCut laser_primary_cut("TOPCAFDigits.m_time>-1300&&TOPCAFDigits.m_time<-900");
  TCut sideband_primary_cut("TOPCAFDigits.m_time>-700&&TOPCAFDigits.m_time<-200");
  //TCut pmt_quality_cut("int((TOPCAFDigits.m_pixel_id-1)/64)==1 && ((TOPCAFDigits.m_pixel_id-1)%64>=16 && (TOPCAFDigits.m_pixel_id-1)%64<24)");
  //TCut pmt_quality_cut("(TOPCAFDigits.m_pixel_id>160 && TOPCAFDigits.m_pixel_id<169) || (TOPCAFDigits.m_pixel_id>64 && TOPCAFDigits.m_pixel_id<73) || (TOPCAFDigits.m_pixel_id>480 && TOPCAFDigits.m_pixel_id<489)");
  //TCut pmt_quality_cut("(TOPCAFDigits.m_pixel_id>80 && TOPCAFDigits.m_pixel_id<89)");
  TCut pmt_quality_cut("(TOPCAFDigits.m_pixel_id<-100)");

  for (int ii = 0; ii < 8; ii++) {
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnum%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut && !pmt_quality_cut);
    t.Draw(Form("(TOPCAFDigits.m_evt_num)>>evtnumq%d", ii), width_cut && qual_cut && cosmic_cut[ii] && height_cut2);
    for (int i = 0; i < 1000; i++) {
      nhit[ii]->Fill(evtnum[ii]->GetBinContent(i));
      nhitq[ii]->Fill(evtnumq[ii]->GetBinContent(i));
    }
  }

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods");
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights", width_cut && qual_cut);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_tdc_bin>>height_time", qual_cut && width_cut);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time", width_cut && qual_cut && height_cut && !pmt_quality_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time_qual", width_cut && qual_cut && height_cut2 && !pmt_quality_cut);
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy", width_cut && qual_cut && cosmic_primary_cut && height_cut2 && !pmt_quality_cut, "COLZ");
  //t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all", width_cut && qual_cut && cosmic_primary_cut && height_cut && !pmt_quality_cut, "COLZ");
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy", width_cut && qual_cut && cosmic_cut[2]
         && height_cut2, "COLZ");
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all", width_cut && qual_cut && cosmic_cut[2]
         && height_cut && !pmt_quality_cut, "COLZ");
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_prism", width_cut && qual_cut && cosmic_cut[4]
         && height_cut2, "COLZ");
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy_all_prism", width_cut && qual_cut && cosmic_cut[4]
         && height_cut, "COLZ");
  t.Draw("TOPCAFDigits.m_pixel_id>>laser_occupancy", width_cut && qual_cut && cosmic_cut[0] && height_cut);


  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy%d", c), width_cut
           && qual_cut && laser_primary_cut && Form("TOPCAFDigits.m_boardstack==%d", c));
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


  pmtxy->SetOption("COLZ");
  pmtxy->Draw();
  pmtxy_all->SetOption("COLZ");
  pmtxy_all->Draw();
  pmtxy_prism->SetOption("COLZ");
  pmtxy_prism->Draw();
  pmtxy_all_prism->SetOption("COLZ");
  pmtxy_all_prism->Draw();


  fout.cd();
  scrods->Write();
  ch_widths->Write();
  ch_heights->Write();
  width_time->Write();
  height_time->Write();
  ch_time->Write();
  ch_time_qual->Write();
  ch_tdcbin->Write();
  tdcbin_time->Write();
  pmtxy->Write();
  pmtxy_all->Write();
  pmtxy_prism->Write();
  pmtxy_all_prism->Write();
  laser_occupancy->Write();
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
  }


  fout.Close();
  return true;

}

int main(int argc, char* argv[])
{
  if (argc == 1) {
    cout << "analyzeHits inputfile.root outputfile.root" << endl;
  } else if (argc == 2) {
    analyzeHits(argv[1]);
  } else if (argc == 3) {
    analyzeHits(argv[1], argv[2]);
  }
  return 0;
}
