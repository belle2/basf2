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
  float occWarning = 1.5;
  float occError = 2;

  //bin 1: occupancy level EMPTY
  ref_occ->SetBinContent(1,occEmpty);

  //bin 2: occupancy level WARNING
  ref_occ->SetBinContent(2,occWarning);

  //bin 3: occupancy level ERROR
  ref_occ->SetBinContent(3,occError);

  TH1F* ref_onlineOcc = new TH1F("refOnlineOccupancy","OnlineOccupancy Levels Reference",3,0,3);

  TString xOnlineLabels[3]={"onlineOccEmpty","onlineOccWarning","onlineOccError"};
  for(int i = 0; i<3; i++)
    ref_onlineOcc->GetXaxis()->SetBinLabel(i+1,xOnlineLabels[i]);

  //infos:
  // onlineOcc < onlineOccEmpty -> black bin
  // onlineOccEmpty <= onlineOcc < onlineOccWarning  -> green bin
  // onlineOccWarning <= onlineOcc < onlineOccError  -> orange bin
  // onlineOcc > onlineOccError  -> red bin

  float onlineOccEmpty = 0;
  float onlineOccWarning = 5;
  float onlineOccError = 10;

  //bin 1: onlineOccupancy level EMPTY
  ref_onlineOcc->SetBinContent(1,onlineOccEmpty);

  //bin 2: onlineOccupancy level WARNING
  ref_onlineOcc->SetBinContent(2,onlineOccWarning);

  //bin 3: onlineOccupancy level ERROR
  ref_onlineOcc->SetBinContent(3,onlineOccError);

  ref_occ->Write();
  ref_onlineOcc->Write();

  f->Close();

}
