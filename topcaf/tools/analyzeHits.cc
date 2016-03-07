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

using namespace std;
using namespace Belle2;

bool analyzeHits(const char* filename)
{
  TChain t("tree");
  t.Add(filename);
  int maxadc = 2000;

  TFile fout("hits.root", "RECREATE");

  TH1F* scrods = new TH1F("scrods", "scrod id numbers (no cuts)", 100, 0., 100.);
  scrods->GetXaxis()->SetTitle("scrod id");

  TH2F* ch_widths = new TH2F("ch_widths", "hit width vs pixel (quality cuts)", 512, 0., 512., 1000, 0., 100.);
  ch_widths->GetXaxis()->SetTitle("pixel");
  ch_widths->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_heights = new TH2F("ch_heights", "hit height vs pixel (quality cuts)", 512, 0., 512., 100, 0., 2048.);
  ch_heights->GetXaxis()->SetTitle("pixel");
  ch_heights->GetYaxis()->SetTitle("hit height [adc bin]");

  TH2F* width_time = new TH2F("width_time", "widths vs times (quality cuts)", 1500, 0., 1500., 1000, 0., 100.);
  width_time->GetXaxis()->SetTitle("hit tdc bin");
  width_time->GetYaxis()->SetTitle("hit width [time bin]");

  TH2F* ch_time = new TH2F("ch_time", "time vs pixel (quality cuts)", 512, 0., 512., 30000, -2000., 2000.);
  ch_time->GetXaxis()->SetTitle("pixel");
  ch_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel (quality cuts)", 512, 0., 512., 1500, 0., (double)maxadc);
  ch_tdcbin->GetXaxis()->SetTitle("pixel");
  ch_tdcbin->GetYaxis()->SetTitle("hit time bin [time bin]");

  TH2F* tdcbin_time = new TH2F("tdcbin_time", "hit time vs tdc bin (quality cuts)", 1500, 0., (double)maxadc, 30000, -2000., 2000.);
  tdcbin_time->GetXaxis()->SetTitle("hit time bin [time bin]");
  tdcbin_time->GetYaxis()->SetTitle("hit time [time bin]");

  TH2F* pmtxy = new TH2F("pmtxy", "pmt hits vs pmt xy position (quality cuts and primary laser times)", 64, 0., 64., 8, 0., 8.);
  pmtxy->GetXaxis()->SetTitle("pmt x");
  pmtxy->GetYaxis()->SetTitle("pmt y");
  pmtxy->SetStats(0);

  TH1F* laser_occupancy = new TH1F("laser_occupancy", "laser hits in each pixel (qual cuts and primary laser times)", 512, 0., 512.);
  laser_occupancy->GetXaxis()->SetTitle("pixel");
  laser_occupancy->GetYaxis()->SetTitle("counts");

  TH1F* asic_occupancy[4];
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c] = new TH1F(Form("asic_occupancy%d", c),
                                 Form("boardstack %d laser hits in each asic (qual cuts and primary laser times)", c), 350, 0., 350.);
    asic_occupancy[c]->GetXaxis()->SetTitle("100 #times carrier + 10 #times asic + asic channel");
    asic_occupancy[c]->GetYaxis()->SetTitle("counts");
  }

  TCut width_cut("TOPCAFDigits.m_width>3.&&TOPCAFDigits.m_width<10.");
  TCut height_cut("TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<2048");
  TCut qual_cut("TOPCAFDigits.m_flag>0&&TOPCAFDigits.m_corr_time!=0");
  TCut laser_primary_cut("TOPCAFDigits.m_time>190&&TOPCAFDigits.m_time<210");

  t.Draw("TOPCAFDigits.m_scrod_id>>scrods");
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_adc_height:TOPCAFDigits.m_pixel_id>>ch_heights", width_cut && qual_cut);
  t.Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time", qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_tdc_bin>>tdcbin_time", width_cut && qual_cut && height_cut);
  t.Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time", width_cut && qual_cut && height_cut);
  t.Draw("(TOPCAFDigits.m_pixel_id-1)/64:(TOPCAFDigits.m_pixel_id-1)%64>>pmtxy", width_cut && qual_cut && laser_primary_cut
         && height_cut);
  t.Draw("TOPCAFDigits.m_pixel_id>>laser_occupancy", width_cut && qual_cut && laser_primary_cut && height_cut);

  for (int c = 0; c < 4; c++) {
    t.Draw(Form("(TOPCAFDigits.m_asic_row*100+TOPCAFDigits.m_asic*10+TOPCAFDigits.m_asic_ch)>>asic_occupancy%d", c), width_cut
           && qual_cut && laser_primary_cut && Form("TOPCAFDigits.m_boardstack==%d", c));
  }

  fout.cd();
  scrods->Write();
  ch_widths->Write();
  ch_heights->Write();
  width_time->Write();
  ch_time->Write();
  ch_tdcbin->Write();
  tdcbin_time->Write();
  pmtxy->Write();
  laser_occupancy->Write();
  for (int c = 0; c < 4; c++) {
    asic_occupancy[c]->Write();
  }

  fout.Close();
  return true;

}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    analyzeHits(argv[1]);
  }

  return 0;
}
