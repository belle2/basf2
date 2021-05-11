/* ARICHValidate.C
 * ROOT macro for ARICH validation plots
 * Author: Luka Santelj
 * 11.3.2014

<header>
<noexecute>Used as library.</noexecute>
</header>
*/

void ARICHValidate(){

  // intput tree
  TChain* ch = new TChain("arich");
  ch->Add("../ARICHEvents.root");

  // output file
  TFile* output = TFile::Open("ARICHValidate.root", "recreate");

  // define histograms for likelihood distributions
  TH1F* hpi = new TH1F("hpi","#pi;L_{#pi}-L_{K}",500000,-200,200);
  TH1F* hk = new TH1F("hk","K;L_{#pi}-L_{K}",500000,-200,200);
  TH1F* hall = new TH1F("hall","Likelihood difference for  K and #pi;L_{#pi}-L_{K}",500,-100,100);

  // plot likelihood difference for K and pi tracks (only tracks within ARICH acceptance and with p>3GeV are included)
  ch->Draw("(logL.pi-logL.K)>>hpi","abs(PDG)==211 && sqrt((mcHit.x - recHit.x)**2 +(mcHit.y - recHit.y)**2)<1 && mcHit.p>3 && status>1000");

  ch->Draw("(logL.pi-logL.K)>>hk","abs(PDG)==321 && sqrt((mcHit.x - recHit.x)**2 +(mcHit.y - recHit.y)**2)<1 && mcHit.p>3 && status>1000");

  ch->Draw("(logL.pi-logL.K)>>hall","(abs(PDG)==321 || abs(PDG)==211) && sqrt((mcHit.x - recHit.x)**2 +(mcHit.y - recHit.y)**2)<1 && mcHit.p>3 && status>1000");

  hall->GetListOfFunctions()->Add(new TNamed("Description", "Difference of ARICHLikelihood value for K and #pi hypothesis, for K and #pi tracks with 3.0 - 3.5 GeV (particle gun from the IP). TrackFinderMCTruth is used for track matching."));
  hall->GetListOfFunctions()->Add(new TNamed("Contact","luka.santelj@ijs.si"));
  hall->GetListOfFunctions()->Add(new TNamed("Check", "Well separated K and #pi peaks."));

  hall->Write();

  TH1F* heff = new TH1F("heff","K id. efficiency vs. #pi missid. probability;fake;efficiency",100,0.005,0.05);

  heff->GetListOfFunctions()->Add(new TNamed("Description", "K identification efficiency vs. #pi missidentification probability, for K and #pi tracks with 3.0 - 3.5 GeV. TrackFinderMCTruth is used for track matching."));
  heff->GetListOfFunctions()->Add(new TNamed("Contact","luka.santelj@ijs.si"));
  heff->GetListOfFunctions()->Add(new TNamed("Check", "Continuous, >0.98 eff. for > 0.02 fake. NOTE: as this is not a usual histogram p-value calculation is not proper so it will often apear RED. If other two plots are OK please don't mind."));

  int startbin = hpi->FindFirstBinAbove(0.);
  int nentrpi = 0;
  int nentrk = hk->Integral(0,startbin-1);
  int allpi = hpi->GetEntries();
  int allk = hk->GetEntries();
  double fakee=0;
  double fake = 0.005;
  int i=0;
  while (fake < 0.05){
    while (fakee < fake){
      nentrpi+=hpi->GetBinContent(startbin);
      nentrk+=hk->GetBinContent(startbin);
      startbin++;
      fakee = double(nentrpi)/double(allpi);
    }
    fake+=0.00045;
    double eff = double(nentrk)/double(allk);
    heff->SetBinContent(i,eff);
    i++;
  }

  heff->Write();

  TH1F* hnphot = new TH1F("hnphot","Number of detected photons;# of photons;Entries", 35,-0.5,34.5);

  ch->Draw("detPhot.pi>>hnphot","abs(PDG)==211 && sqrt((mcHit.x - recHit.x)**2 +(mcHit.y - recHit.y)**2)<1 && mcHit.p>3 && status>1000");

  hnphot->GetListOfFunctions()->Add(new TNamed("Description", "Number of detected photons in a 3#sigma band around the expected Cherenkov angle, for #pi with momenta 3.0-3.5 GeV."));
  hnphot->GetListOfFunctions()->Add(new TNamed("Contact","luka.santelj@ijs.si"));
  hnphot->GetListOfFunctions()->Add(new TNamed("Check", "Poissonian peak with average at ~11.5."));

  hnphot->Write();

  output->Close();

  delete output;

}
