/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
{
  TString sample = "phase2.1.3_june_Touschek/EM_2/";
  TString outfile = "phase2.1.3_june_Touschek_EM_2.root";
  TString folder = "/group/belle2/BGcampaigns/g4sim/phase2.1.3_june_Touschek/EM_2/beast_hist/";
  TFile* ff[6];
  ff[0] = new TFile(TString::Format("%s/Touschek_LER_all.root",folder.Data()));
  ff[1] = new TFile(TString::Format("%s/Coulomb_LER_all.root",folder.Data()));  
  ff[2] = new TFile(TString::Format("%s/Touschek_HER_all.root",folder.Data()));
  ff[3] = new TFile(TString::Format("%s/Coulomb_HER_all.root",folder.Data()));
  ff[4] = new TFile(TString::Format("%s/Brems_HER_all.root",folder.Data()));
  ff[5] = new TFile(TString::Format("%s/Brems_LER_all.root",folder.Data()));
  int tk[6] = {0,0,1,1,0,0};
  TString nn[6] = {"Touschek_LER","Coulomb_LER","Touschek_HER","Coulomb_HER","Brems_HER","Brems_LER"};	
  
  TFile* fout = new TFile(outfile,"RECREATE"); 
  
  std::vector<TH1F*> hists[6];
  for(int k=0;k<6;k++){
    if(!tk[k]) continue;
    hists[k].push_back((TH1F*)ff[k]->Get("NeutronHits"));
    hists[k].push_back((TH1F*)ff[k]->Get("h_mctpc_recoil_w"));
    hists[k].push_back((TH1F*)ff[k]->Get("h_mctpc_recoil"));
    for(int i=0;i<8;i++){
      hists[k].push_back((TH1F*)ff[k]->Get(TString::Format("dia_dose_%d", i)));
    }
    for(int i=0;i<16;i++){
      hists[k].push_back((TH1F*)ff[k]->Get(TString::Format("clawss_rate1_%d", i)));
    }
    hists[k].push_back((TH1F*)ff[k]->Get("h_pxNb"));
    fout->cd();
    for(auto hist : hists[k]){ 
      hist->SetName(nn[k]+"_"+hist->GetName());
      hist->Write();
    }
  }
  fout->Close();

}
