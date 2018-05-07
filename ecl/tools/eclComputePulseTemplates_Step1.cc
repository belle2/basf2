#include <TF1.h>
#include <ecl/digitization/OfflineFitFunction.h>
#include <TTree.h>
#include <TFile.h>
#include <TGraph.h>
#include <iostream>
#include <assert.h>

//
/*
 See eclComputePulseTemplates_Step0.cc for README instructions.
 */
//
int main(int argc, char* argv[])
{
  //
  TString OutputDirectory = "";
  if (OutputDirectory == "") {
    std::cout << "Error set ouput directory" << std::endl;
    return -1;
  }
  //
  assert(argc == 3);
  int LowIDLimit = atoi(argv[1]);
  int HighIDLimit = atoi(argv[2]);
  //
  double PhotonWaveformPar[11];
  TFile* PhotonParFile = new TFile(OutputDirectory + "PhotonWaveformParameters.root");
  TTree* chain = (TTree*) PhotonParFile->Get("ParTree");
  chain->SetBranchAddress("PhotonPar", &PhotonWaveformPar);
  //
  TFile* f = new TFile(OutputDirectory + Form("PhotonShapes_Low%d_High%d.root", LowIDLimit, HighIDLimit), "RECREATE");
  f->cd();
  TTree* mtree = new TTree("mtree", "");
  std::vector<double> PhotonWaveformArray(100000);
  mtree->Branch("PhotonArray", PhotonWaveformArray.data(), "PhotonWaveformArray[100000]/D");

  //
  for (Long64_t jentry = LowIDLimit; jentry < HighIDLimit; jentry++) {
    chain->GetEntry(jentry);
    TF1 PhotonShapeFunc = TF1(Form("photonShape_%lld", jentry), Belle2::ECL::WaveFuncTwoComponent, 0, 20, 26);;
    PhotonShapeFunc.SetNpx(1000);
    std::cout << PhotonWaveformPar[0] << std::endl;
    PhotonShapeFunc.SetParameter(0, 0);
    PhotonShapeFunc.SetParameter(1, 0);
    PhotonShapeFunc.SetParameter(2, 1);
    PhotonShapeFunc.SetParameter(3, 0);
    for (int k = 0; k < 10; k++) {
      PhotonShapeFunc.SetParameter(4 + k, PhotonWaveformPar[k + 1]);
      PhotonShapeFunc.SetParameter(10 + 4 + k, PhotonWaveformPar[k + 1]);
    }
    PhotonShapeFunc.SetParameter(24, PhotonWaveformPar[0]);
    PhotonShapeFunc.SetParameter(25, 1);
    //
    if (PhotonWaveformPar[0] > 0 && jentry <= 8737) {
      for (int k = 0; k < 100000; k++) PhotonWaveformArray[k] = PhotonShapeFunc.Eval(((double)k) * (1. / 1000.)) ;
    } else {
      for (int k = 0; k < 100000; k++) PhotonWaveformArray[k] = -999;
    }
    mtree->Fill();
    //
  }
  //
  f->cd();
  mtree->Write();
  f->Write();
  //
  return 0;
}
