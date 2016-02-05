#include <TFile.h>
#include <TTree.h>
#include <TCut.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TH1F.h>
#include <TH2F.h>
#include <iostream>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

bool analyzeHits(const char* filename)
{

  TFile fileIn(filename);

  TTree* t;

  fileIn.GetObject("tree", t);

  TFile fout("hits.root", "RECREATE");

  TH1F* scrods = new TH1F("scrods", "scrod id numbers", 100, 0., 100.);
  TH2F* ch_widths = new TH2F("ch_widths", "ch_widths", 512, 0., 512., 1000, 0., 100.);
  TH2F* width_time = new TH2F("width_time", "widths vs times", 1500, 0., 1500., 1000, 0., 100.);
  TH2F* ch_time = new TH2F("ch_time", "time vs channel", 512, 0., 512., 30000, -1500., 1500.);
  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel", 512, 0., 512., 1500, 0., 1500.);

  TCut width_cut("TOPCAFDigits.m_width>1.&&TOPCAFDigits.m_width<6.");
  //  TCut qual_cut("TOPCAFDigits.m_tdc_bin<1200&&TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<500&&TOPCAFDigits.m_flag>0&&TOPCAFDigits.m_corr_time>40");
  TCut qual_cut("TOPCAFDigits.m_tdc_bin<1200&&TOPCAFDigits.m_adc_height>50&&TOPCAFDigits.m_adc_height<500&&TOPCAFDigits.m_flag>0");

  t->Draw("TOPCAFDigits.m_scrod_id>>scrods");
  t->Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_pixel_id>>ch_widths", qual_cut);
  t->Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time", qual_cut);
  t->Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin", width_cut && qual_cut);
  t->Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time", width_cut && qual_cut);


  fout.cd();
  scrods->Write();
  ch_widths->Write();
  width_time->Write();
  ch_time->Write();
  ch_tdcbin->Write();


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
