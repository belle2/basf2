/******************************************************************
 * simple macro to create reference plots for the 
 * DQM analsyis module:
 *
 *    DQMHistAnalysisSVDGeneral
 * 
 * content:
 * 1. reference plot to set Occupancy Levels (Name = refOccupancy)
 *
 * usage:
 * > root -l createSVDreference.C 
 * will create the root file SVDrefHisto.root contaninig the references
 *
 *******************************************************************/

void createSVDreference(){

  //file containing references plots 

  TFile *f = new TFile("SVDrefHisto.root", "RECREATE");

  TH1F* ref_occ = new TH1F("refOccupancy","Occupancy Levels Reference",3,0,3);

  TString xLabels[3]={"occEmpty","occWarning","occError"};
  for(int i = 0; i<3; i++)
    ref_occ->GetXaxis()->SetBinLabel(i+1,xLabels[i]);

  //infos:
  // occ < occEmpty -> black bin
  // occEmpty <= occ < occWarning  -> green bin
  // occWarning <= occ < occError  -> orange bin
  // occ > occError  -> red bin

  float occEmpty = 0;
  float occWarning = 3;
  float occError = 5;

  //bin 1: occupancy level EMPTY
  ref_occ->SetBinContent(1,occEmpty);

  //bin 2: occupancy level WARNING
  ref_occ->SetBinContent(2,occWarning);

  //bin 3: occupancy level ERROR
  ref_occ->SetBinContent(3,occError);

  ref_occ->Write();

  f->Close();

}
