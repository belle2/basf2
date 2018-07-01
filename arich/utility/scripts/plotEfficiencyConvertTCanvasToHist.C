/**
 * This script converts TCanvas with ARICH efficiency produced with plotEfficiency.C
 * to simple root files with histograms.
 *
 * run as: "root -l ../../examples/ARICHPerformance.root plotEfficiencyConvertTCanvasToHist.C"
 *
 * Author: Leonid Burmistrov, 06.06.2018
 **/

Int_t plotEfficiencyConvertTCanvasToHist(){

  TH2F *hh = new TH2F();
  TH1F *h_eff3 = new TH1F();
  TH1F *h_eff2 = new TH1F();
  TH1F *h_eff1 = new TH1F();
  TH2F *h_trk = new TH2F();
  TH1F *h_momPi = new TH1F();
  TH1F *h_momK = new TH1F();
  TH1F *h_thetaPi = new TH1F();
  TH1F *h_thetaK = new TH1F();
  TH1F *h_nphotPi = new TH1F();
  TH1F *h_nphotK = new TH1F();
  TH1F *h_chPi = new TH1F();
  TH1F *h_chK = new TH1F();
  TH1F *h_lkhPi = new TH1F();
  TH1F *h_lkhK = new TH1F();

  TCanvas *c1 = (TCanvas*)_file0->Get("c1");
  TObject *obj;
  TObject *obj2;
  TIter next(c1->GetListOfPrimitives());
  while ((obj=next())) {
    //cout << "--------------------------" << endl;
    //cout << "Reading: " << obj->GetName() << endl;
    //obj->Info("","");
    //if (obj->InheritsFrom("TH1")) {
    //cout << "histo: " << obj->GetName() << endl;
    //}
    if (obj->InheritsFrom("TPad")) {
      //cout << "     TPad: " << obj->GetName() << endl;
      TPad *p1 = (TPad*)obj;
      TIter nexttonext(p1->GetListOfPrimitives());
      while ((obj2=nexttonext())) {
	TString obj2Name = obj2->GetName();
	//cout << "          Reading: " << obj2Name << endl;
	//obj2->Info("","");	
	if(obj2Name == "hh")
	  hh = (TH2F*)obj2;	  
	if(obj2Name == "h_eff3")
	    h_eff3 = (TH1F*)obj2;
	if(obj2Name == "h_eff2")
	  h_eff2 = (TH1F*)obj2;
	if(obj2Name == "h_eff1")
	  h_eff1 = (TH1F*)obj2;
	if(obj2Name == "h_trk")
	  h_trk = (TH2F*)obj2;
	if(obj2Name == "h_momPi")
	  h_momPi = (TH1F*)obj2;
	if(obj2Name == "h_momK")
	  h_momK = (TH1F*)obj2;
	if(obj2Name == "h_thetaPi")
	  h_thetaPi = (TH1F*)obj2;
	if(obj2Name == "h_thetaK")
	  h_thetaK = (TH1F*)obj2;
	if(obj2Name == "h_nphotPi")
	  h_nphotPi = (TH1F*)obj2;
	if(obj2Name == "h_nphotK")
	  h_nphotK = (TH1F*)obj2;
	if(obj2Name == "h_chPi")
	  h_chPi = (TH1F*)obj2;
	if(obj2Name == "h_chK")
	  h_chK = (TH1F*)obj2;
	if(obj2Name == "h_lkhPi")
	  h_lkhPi = (TH1F*)obj2;
	if(obj2Name == "h_lkhK")
	  h_lkhK = (TH1F*)obj2;
      }
    }
    //cout << "--------------------------" << endl;
  }  

  TString outFname = _file0->GetName();
  outFname += "_histo.root";
  cout<<"outFname "<<outFname<<endl;
  TFile* rootFile = new TFile(outFname.Data(), "RECREATE", " Histograms", 1);

  hh->Write();
  h_eff3->Write();
  h_eff2->Write();
  h_eff1->Write();
  h_trk->Write();
  h_momPi->Write();
  h_momK->Write();
  h_thetaPi->Write();
  h_thetaK->Write();  
  h_nphotPi->Write();
  h_nphotK->Write();  
  h_chPi->Write();
  h_chK->Write();  
  h_lkhPi->Write();
  h_lkhK->Write();
  
  return 0;
}
