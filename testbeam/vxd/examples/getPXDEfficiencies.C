{ 

gROOT->Reset(); 
gROOT->SetStyle("Plain"); 

gStyle->SetOptFit();
gStyle->SetPalette(1);
gStyle->SetCanvasColor(0);
gStyle->SetTitleFillColor(0);
gStyle->SetPaintTextFormat("1f");
gStyle->SetOptStat(1111);

 gStyle->SetStatY(0.97);                
 // Set y-position (fraction of pad size)
 gStyle->SetStatX(0.97);                
 // Set x-position (fraction of pad size)
 gStyle->SetStatW(0.35);                
 // Set width of stat-box (fraction of pad size)
 gStyle->SetStatH(0.18);                
 // Set height of stat-box (fraction of pad size)

 //Set this to true and set a plotdir to save generated plots automatically
 bool savePlots = false;
 TString plotBaseName = "plotdir/plot_";

 // event data
 TFile *ftb = new TFile("efficiency.root");

TCut qc ="fit_mom>1 && fit_mom < 8 && fit_pValue > 0.01"; 

//To select only a region 
TCut region_v1 =" vcell_fit_1_1_2 < 1000 && vcell_fit_1_1_2 > 0 "; 
TCut region_v2 =" vcell_fit_2_1_2 < 1000 && vcell_fit_2_1_2 > 0 "; 
TCut region_u1 =" ucell_fit_1_1_2 < 1000 && ucell_fit_1_1_2 > 0 "; 
TCut region_u2 =" ucell_fit_2_1_2 < 1000 && ucell_fit_2_1_2 > 0 "; 

//Run 176 area of highest efficiency
//region_v1 =" vcell_fit_1_1_2 < 200 && vcell_fit_1_1_2 > 100 "; 
//region_v2 =" vcell_fit_2_1_2 < 1000 && vcell_fit_2_1_2 > 500 "; 
//region_u1 =" ucell_fit_1_1_2 < 1000 && ucell_fit_1_1_2 > 0 "; 
//region_u2 =" ucell_fit_2_1_2 < 1000 && ucell_fit_2_1_2 > 0 "; 



TCut roi_1 ="roi_number_of_1_1_2 == 1 && roi_fit_inside_1_1_2 "; 
TCut roi_2 ="roi_number_of_2_1_2 == 1 && roi_fit_inside_2_1_2 ";

//roi_1 ="";
//roi_2 ="";


TTree *ttbtrack = (TTree*) ftb->Get("PXDEffiTree");

TCanvas * c001  = new TCanvas("c001","c001",600,400);
c001->SetLeftMargin(0.2);
c001->SetRightMargin(0.2);
c001->SetTopMargin(0.1);
c001->SetBottomMargin(0.16);

ttbtrack->Draw("u_clus_1_1_2 >>ht_l1" , "cluster_matched_1_1_2==0" && qc && roi_1  ,"profz");
ht_l1->SetName("ht_l1"); 
ht_l1->SetTitle("");
ht_l1->GetXaxis()->SetTitle("uCell Sensor 1.1.2");
ht_l1->GetYaxis()->SetTitle("number of clusters");
ht_l1->GetXaxis()->SetTitleSize(0.08);
ht_l1->GetYaxis()->SetTitleSize(0.08);
ht_l1->GetXaxis()->SetLabelSize(0.08);
ht_l1->GetYaxis()->SetLabelSize(0.08);
ht_l1->Draw("colz");
if (savePlots) c001->Print(TString::Format("%snum_clus_u1.pdf",plotBaseName.Data()));


TCanvas * c002  = new TCanvas("c002","c002",600,400);
c002->SetLeftMargin(0.2);
c002->SetRightMargin(0.2);
c002->SetTopMargin(0.1);
c002->SetBottomMargin(0.16);

ttbtrack->Draw("u_clus_2_1_2 >>ht_l2" , "cluster_matched_2_1_2==0" && qc && roi_2  ,"profz");
ht_l2->SetName("ht_l2"); 
ht_l2->SetTitle("");
ht_l2->GetXaxis()->SetTitle("uCell Sensor 2.1.2");
ht_l2->GetYaxis()->SetTitle("number of clusters");
ht_l2->GetXaxis()->SetTitleSize(0.08);
ht_l2->GetYaxis()->SetTitleSize(0.08);
ht_l2->GetXaxis()->SetLabelSize(0.08);
ht_l2->GetYaxis()->SetLabelSize(0.08);
ht_l2->Draw("colz");
if (savePlots) c002->Print(TString::Format("%snum_clus_u2.pdf",plotBaseName.Data()));



TCanvas * c003  = new TCanvas("c003","c003",600,400);
c003->SetLeftMargin(0.2);
c003->SetRightMargin(0.1);
c003->SetTopMargin(0.1);
c003->SetBottomMargin(0.16);

ttbtrack->Draw("(u_fit_1_1_2 - u_clus_1_1_2)*10000 >>hu(601,-475.,+475.)", "cluster_matched_1_1_2==0" && qc && roi_1);
hu->SetName("Residuals"); 
hu->SetTitle("");
hu->GetXaxis()->SetTitle("residuals u [#mum]");
hu->GetYaxis()->SetTitle("number of hits");
hu->GetXaxis()->SetTitleSize(0.06);
hu->GetYaxis()->SetTitleSize(0.06);
hu->GetYaxis()->SetTitleOffset(1.5);
hu->GetXaxis()->SetLabelSize(0.06);
hu->GetYaxis()->SetLabelSize(0.06);
hu->Draw("");
if (savePlots) c003->Print(TString::Format("%sresiduals_u1.pdf",plotBaseName.Data()));



TCanvas * c0031  = new TCanvas("c0031","c0031",600,400);
c0031->SetLeftMargin(0.2);
c0031->SetRightMargin(0.1);
c0031->SetTopMargin(0.1);
c0031->SetBottomMargin(0.16);

ttbtrack->Draw("(v_fit_1_1_2 - v_clus_1_1_2)*10000 >>hv(601,-475.,+475.)", "cluster_matched_1_1_2==0" && qc && roi_1);
hv->SetName("Residuals"); 
hv->SetTitle("");
hv->GetXaxis()->SetTitle("residuals v [#mum]");
hv->GetYaxis()->SetTitle("number of hits");
hv->GetXaxis()->SetTitleSize(0.06);
hv->GetYaxis()->SetTitleSize(0.06);
hv->GetYaxis()->SetTitleOffset(1.5);
hv->GetXaxis()->SetLabelSize(0.06);
hv->GetYaxis()->SetLabelSize(0.06);
hv->Draw("");
if (savePlots) c0031->Print(TString::Format("%sresiduals_v1.pdf",plotBaseName.Data()));



TCanvas * c0032  = new TCanvas("c0032","c0032",600,400);
c0032->SetLeftMargin(0.2);
c0032->SetRightMargin(0.1);
c0032->SetTopMargin(0.1);
c0032->SetBottomMargin(0.16);

ttbtrack->Draw("(u_fit_2_1_2 - u_clus_2_1_2)*10000 >>hu2(601,-475.,+475.)", "cluster_matched_2_1_2==0" && qc && roi_2);
hu2->SetName("Residuals"); 
hu2->SetTitle("");
hu2->GetXaxis()->SetTitle("residuals u [#mum]");
hu2->GetYaxis()->SetTitle("number of hits");
hu2->GetXaxis()->SetTitleSize(0.06);
hu2->GetYaxis()->SetTitleSize(0.06);
hu2->GetYaxis()->SetTitleOffset(1.5);
hu2->GetXaxis()->SetLabelSize(0.06);
hu2->GetYaxis()->SetLabelSize(0.06);
hu2->Draw("");
if (savePlots) c0032->Print(TString::Format("%sresiduals_u2.pdf",plotBaseName.Data()));



TCanvas * c0038  = new TCanvas("c0038","c0038",600,400);
c0038->SetLeftMargin(0.2);
c0038->SetRightMargin(0.1);
c0038->SetTopMargin(0.1);
c0038->SetBottomMargin(0.16);

ttbtrack->Draw("clus_charge_1_1_2 >>hch1(100,0,100.)", "clus_charge_1_1_2>=0 && clus_charge_1_1_2<100" && qc);
hch1->SetName("Cluster Charge");
hch1->SetTitle("");
hch1->GetXaxis()->SetTitle("Cluster Charge");
hch1->GetYaxis()->SetTitle("number of hits");
hch1->GetXaxis()->SetTitleSize(0.06);
hch1->GetYaxis()->SetTitleSize(0.06);
hch1->GetYaxis()->SetTitleOffset(1.5);
hch1->GetXaxis()->SetLabelSize(0.06);
hch1->GetYaxis()->SetLabelSize(0.06);
hch1->Draw("");
if (savePlots) c0038->Print(TString::Format("%scharge_1.pdf",plotBaseName.Data()));

TCanvas * c0039  = new TCanvas("c0039","c0039",600,400);
c0039->SetLeftMargin(0.2);
c0039->SetRightMargin(0.1);
c0039->SetTopMargin(0.1);
c0039->SetBottomMargin(0.16);

ttbtrack->Draw("clus_charge_2_1_2 >>hch2(100,0,100)", "clus_charge_2_1_2>=0 && clus_charge_2_1_2<100" && qc);
hch2->SetName("Cluster Charge");
hch2->SetTitle("");
hch2->GetXaxis()->SetTitle("Cluster Charge");
hch2->GetYaxis()->SetTitle("number of hits");
hch2->GetXaxis()->SetTitleSize(0.06);
hch2->GetYaxis()->SetTitleSize(0.06);
hch2->GetYaxis()->SetTitleOffset(1.5);
hch2->GetXaxis()->SetLabelSize(0.06);
hch2->GetYaxis()->SetLabelSize(0.06);
hch2->Draw("");
if (savePlots) c0039->Print(TString::Format("%scharge_2.pdf",plotBaseName.Data()));



TCanvas * c0033  = new TCanvas("c0033","c0033",600,400);
c0033->SetLeftMargin(0.2);
c0033->SetRightMargin(0.1);
c0033->SetTopMargin(0.1);
c0033->SetBottomMargin(0.16);

ttbtrack->Draw("(v_fit_2_1_2 - v_clus_2_1_2)*10000 >>hv2(601,-475.,+475.)", "cluster_matched_2_1_2==0" && qc && roi_2);
hv2->SetName("Residuals"); 
hv2->SetTitle("");
hv2->GetXaxis()->SetTitle("residuals v [#mum]");
hv2->GetYaxis()->SetTitle("number of hits");
hv2->GetXaxis()->SetTitleSize(0.06);
hv2->GetYaxis()->SetTitleSize(0.06);
hv2->GetYaxis()->SetTitleOffset(1.5);
hv2->GetXaxis()->SetLabelSize(0.06);
hv2->GetYaxis()->SetLabelSize(0.06);
hv2->Draw("");
if (savePlots) c0033->Print(TString::Format("%sresiduals_v2.pdf",plotBaseName.Data()));




TCanvas * c0  = new TCanvas("c0","c0",600,400);
c0->SetLeftMargin(0.2);
c0->SetRightMargin(0.2);
c0->SetTopMargin(0.1);
c0->SetBottomMargin(0.16);
ttbtrack->Draw("vcell_fit_1_1_2:ucell_fit_1_1_2>>hspot_1_1_2(40,0,250,200,0,768)" , ""  ,"colz");
hspot_1_1_2->SetName("hspot_1_1_2"); 
hspot_1_1_2->SetTitle("");
hspot_1_1_2->SetStats( false );
hspot_1_1_2->GetXaxis()->SetTitle("uCell Sensor 1.1.2");
hspot_1_1_2->GetYaxis()->SetTitle("vCell Sensor 1.1.2");
hspot_1_1_2->GetZaxis()->SetTitle("number of tracks");
hspot_1_1_2->GetXaxis()->SetTitleSize(0.08);
hspot_1_1_2->GetYaxis()->SetTitleSize(0.08);
hspot_1_1_2->GetZaxis()->SetTitleSize(0.08);
hspot_1_1_2->GetXaxis()->SetLabelSize(0.08);
hspot_1_1_2->GetYaxis()->SetLabelSize(0.08);
hspot_1_1_2->GetZaxis()->SetLabelSize(0.08);
hspot_1_1_2->Draw("colz");
if (savePlots) c0->Print(TString::Format("%sbeamspot_1.pdf",plotBaseName.Data()));



TCanvas * c01  = new TCanvas("c01","c01",600,400);
c01->SetLeftMargin(0.2);
c01->SetRightMargin(0.2);
c01->SetTopMargin(0.1);
c01->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_fit_2_1_2:ucell_fit_2_1_2>>hspot_2_1_2(40,0,250,200,0,768)", "" , "colz");
hspot_2_1_2->SetName("hspot_2_1_2"); 
hspot_2_1_2->SetTitle("");
hspot_2_1_2->SetStats( false );  
hspot_2_1_2->GetXaxis()->SetTitle("uCell Sensor 2.1.2");
hspot_2_1_2->GetYaxis()->SetTitle("vCell Sensor 2.1.2");
hspot_2_1_2->GetZaxis()->SetTitle("number of tracks");
hspot_2_1_2->GetXaxis()->SetTitleSize(0.08);
hspot_2_1_2->GetYaxis()->SetTitleSize(0.08);
hspot_2_1_2->GetZaxis()->SetTitleSize(0.08);
hspot_2_1_2->GetXaxis()->SetLabelSize(0.08);
hspot_2_1_2->GetYaxis()->SetLabelSize(0.08);
hspot_2_1_2->GetZaxis()->SetLabelSize(0.08);
hspot_2_1_2->Draw("colz");
if (savePlots) c01->Print(TString::Format("%sbeamspot_2.pdf",plotBaseName.Data()));



TCanvas * c02  = new TCanvas("c02","c02",600,400);
c02->SetLeftMargin(0.2);
c02->SetRightMargin(0.1);
c02->SetTopMargin(0.1);
c02->SetBottomMargin(0.16);

ttbtrack->Draw("fit_pValue >> hpavlue(100,0,1)");
hpavlue->SetName("hpavlue"); 
hpavlue->SetTitle("");
hpavlue->GetXaxis()->SetTitle("fit p-value");
hpavlue->GetYaxis()->SetTitle("number of tracks");
hpavlue->GetXaxis()->SetTitleSize(0.06);
hpavlue->GetYaxis()->SetTitleSize(0.06);
hpavlue->GetYaxis()->SetTitleOffset(1.5);
hpavlue->GetXaxis()->SetLabelSize(0.06);
hpavlue->GetYaxis()->SetLabelSize(0.06);
hpavlue->Draw();
if (savePlots) c02->Print(TString::Format("%sp_value.pdf",plotBaseName.Data()));



TCanvas * c03  = new TCanvas("c03","c03",600,400);
c03->SetLeftMargin(0.2);
c03->SetRightMargin(0.1);
c03->SetTopMargin(0.1);
c03->SetBottomMargin(0.16);

ttbtrack->Draw("fit_mom >> hmom(100,0,8)");
hmom->SetName("hmom"); 
hmom->SetTitle("");
hmom->GetXaxis()->SetTitle("fit momentum [GeV]");
hmom->GetYaxis()->SetTitle("number of tracks");
hmom->GetXaxis()->SetTitleSize(0.06);
hmom->GetYaxis()->SetTitleSize(0.06);
hmom->GetYaxis()->SetTitleOffset(1.5);
hmom->GetXaxis()->SetLabelSize(0.06);
hmom->GetYaxis()->SetLabelSize(0.06);
hmom->Draw();
if (savePlots) c03->Print(TString::Format("%sfit_mom.pdf",plotBaseName.Data()));



TCanvas * c2  = new TCanvas("c2","c2",600,400);
c2->SetLeftMargin(0.2);
c2->SetRightMargin(0.1);
c2->SetTopMargin(0.1);
c2->SetBottomMargin(0.16);

ttbtrack->Draw(" ucell_fit_1_1_2 >> htotal_ucell_1_1_2(10,0,250)" , qc && roi_1 && region_v1 );
ttbtrack->Draw(" ucell_fit_1_1_2 >> hmatch_ucell_1_1_2(10,0,250)" , "cluster_matched_1_1_2==0" && qc && roi_1 && region_v1);

TH1F* match_ucell_1_1_2 = dynamic_cast<TH1F*> (hmatch_ucell_1_1_2); 
TH1F* total_ucell_1_1_2 = dynamic_cast<TH1F*> (htotal_ucell_1_1_2); 

 TGraphAsymmErrors * effi_ucell_1_1_2 = new TGraphAsymmErrors ( match_ucell_1_1_2, total_ucell_1_1_2, "w" ); 

effi_ucell_1_1_2->SetTitle("");
effi_ucell_1_1_2->GetXaxis()->SetTitle("u position [pitch units]");
effi_ucell_1_1_2->GetYaxis()->SetTitle("average hit efficiency");
effi_ucell_1_1_2->GetXaxis()->SetRangeUser(0,250);
effi_ucell_1_1_2->GetYaxis()->SetRangeUser(0,1.0);
effi_ucell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effi_ucell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effi_ucell_1_1_2->GetYaxis()->SetTitleOffset(1.7);

effi_ucell_1_1_2->SetMarkerColor(kBlack);
effi_ucell_1_1_2->SetLineColor(kBlack);
effi_ucell_1_1_2->SetMarkerStyle(22);

ttbtrack->Draw(" ucell_fit_2_1_2 >> htotal_ucell_2_1_2(10,0,250)" , qc && roi_2 && region_v2);
ttbtrack->Draw(" ucell_fit_2_1_2 >> hmatch_ucell_2_1_2(10,0,250)" , "cluster_matched_2_1_2==0" && qc && roi_2 && region_v2 );


TH1F* match_ucell_2_1_2 = dynamic_cast<TH1F*> (hmatch_ucell_2_1_2); 
TH1F* total_ucell_2_1_2 = dynamic_cast<TH1F*> (htotal_ucell_2_1_2); 

TGraphAsymmErrors * effi_ucell_2_1_2 = new TGraphAsymmErrors ( match_ucell_2_1_2, total_ucell_2_1_2, "w" ); 

effi_ucell_2_1_2->SetTitle("");
effi_ucell_2_1_2->GetXaxis()->SetTitle("u position [pitch units]");
effi_ucell_2_1_2->GetYaxis()->SetTitle("average hit efficiency");
effi_ucell_2_1_2->GetXaxis()->SetRangeUser(0,250);
effi_ucell_2_1_2->GetYaxis()->SetRangeUser(0,1.0);//0.9,1
effi_ucell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effi_ucell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effi_ucell_2_1_2->GetYaxis()->SetTitleOffset(1.7);

effi_ucell_2_1_2->SetMarkerColor(kBlue);
effi_ucell_2_1_2->SetLineColor(kBlue);
effi_ucell_2_1_2->SetMarkerStyle(25);

effi_ucell_2_1_2->Draw(); 
effi_ucell_1_1_2->Draw("same"); 

TLegend* lc = new TLegend(0.6,0.2,0.85,0.4);
lc->SetFillColor(kWhite); 
lc->SetBorderSize(0);
lc->SetTextSize(0.06);
lc->AddEntry(effi_ucell_1_1_2,"PXD Layer 1","lep");
lc->AddEntry(effi_ucell_2_1_2,"PXD Layer 2","lep");
lc->Draw();
if (savePlots) c2->Print(TString::Format("%seff_u.pdf",plotBaseName.Data()));




TCanvas * c2a  = new TCanvas("c2a","c2a",600,400);
c2a->SetLeftMargin(0.2);
c2a->SetRightMargin(0.1);
c2a->SetTopMargin(0.1);
c2a->SetBottomMargin(0.16);

ttbtrack->Draw(" ucell_fit_1_1_2 >> htotalo_ucell_1_1_2(10,0,250)" , "otherpxd_cluster_matched_1_1_2 == 0" && qc && roi_1 && region_v1 );
ttbtrack->Draw(" ucell_fit_1_1_2 >> hmatcho_ucell_1_1_2(10,0,250)" , "otherpxd_cluster_matched_1_1_2 == 0 && cluster_matched_1_1_2==0" && qc && roi_1 && region_v1);

TH1F* matcho_ucell_1_1_2 = dynamic_cast<TH1F*> (hmatcho_ucell_1_1_2); 
TH1F* totalo_ucell_1_1_2 = dynamic_cast<TH1F*> (htotalo_ucell_1_1_2); 

 TGraphAsymmErrors * effio_ucell_1_1_2 = new TGraphAsymmErrors ( matcho_ucell_1_1_2, totalo_ucell_1_1_2, "w" ); 

effio_ucell_1_1_2->SetTitle("");
effio_ucell_1_1_2->GetXaxis()->SetTitle("u position [pitch units]");
effio_ucell_1_1_2->GetYaxis()->SetTitle("avg. hit eff., hit on other pxd");
effio_ucell_1_1_2->GetXaxis()->SetRangeUser(0,250);
effio_ucell_1_1_2->GetYaxis()->SetRangeUser(0,1.0);
effio_ucell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effio_ucell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effio_ucell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effio_ucell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effio_ucell_1_1_2->GetYaxis()->SetTitleOffset(1.7);

effio_ucell_1_1_2->SetMarkerColor(kBlack);
effio_ucell_1_1_2->SetLineColor(kBlack);
effio_ucell_1_1_2->SetMarkerStyle(22);

ttbtrack->Draw(" ucell_fit_2_1_2 >> htotalo_ucell_2_1_2(10,0,250)" , "otherpxd_cluster_matched_2_1_2 == 0" && qc && roi_2 && region_v2);
ttbtrack->Draw(" ucell_fit_2_1_2 >> hmatcho_ucell_2_1_2(10,0,250)" , "otherpxd_cluster_matched_2_1_2 == 0 && cluster_matched_2_1_2==0" && qc && roi_2 && region_v2 );


TH1F* matcho_ucell_2_1_2 = dynamic_cast<TH1F*> (hmatcho_ucell_2_1_2); 
TH1F* totalo_ucell_2_1_2 = dynamic_cast<TH1F*> (htotalo_ucell_2_1_2); 

TGraphAsymmErrors * effio_ucell_2_1_2 = new TGraphAsymmErrors ( matcho_ucell_2_1_2, totalo_ucell_2_1_2, "w" ); 

effio_ucell_2_1_2->SetTitle("");
effio_ucell_2_1_2->GetXaxis()->SetTitle("u position [pitch units]");
effio_ucell_2_1_2->GetYaxis()->SetTitle("avg. hit eff., hit on other pxd");
effio_ucell_2_1_2->GetXaxis()->SetRangeUser(0,250);
effio_ucell_2_1_2->GetYaxis()->SetRangeUser(0,1.0);//0.9,1
effio_ucell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effio_ucell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effio_ucell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effio_ucell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effio_ucell_2_1_2->GetYaxis()->SetTitleOffset(1.7);

effio_ucell_2_1_2->SetMarkerColor(kBlue);
effio_ucell_2_1_2->SetLineColor(kBlue);
effio_ucell_2_1_2->SetMarkerStyle(25);

effio_ucell_2_1_2->Draw(); 
effio_ucell_1_1_2->Draw("same"); 

TLegend* lco = new TLegend(0.6,0.2,0.85,0.4);
lco->SetFillColor(kWhite); 
lco->SetBorderSize(0);
lco->SetTextSize(0.06);
lco->AddEntry(effi_ucell_1_1_2,"PXD Layer 1","lep");
lco->AddEntry(effi_ucell_2_1_2,"PXD Layer 2","lep");
lco->Draw();
if (savePlots) c2a->Print(TString::Format("%seff_u_other.pdf",plotBaseName.Data()));



TCanvas * c21  = new TCanvas("c21","c21",600,400);
c21->SetLeftMargin(0.2);
c21->SetRightMargin(0.1);
c21->SetTopMargin(0.1);
c21->SetBottomMargin(0.16);

ttbtrack->Draw(" vcell_fit_1_1_2 >> htotal_vcell_1_1_2(10,0,768)" , qc && roi_1 && region_u1);
ttbtrack->Draw(" vcell_fit_1_1_2 >> hmatch_vcell_1_1_2(10,0,768)" , "cluster_matched_1_1_2==0 " && qc && roi_1 && region_u1);

TH1F* match_vcell_1_1_2 = dynamic_cast<TH1F*> (hmatch_vcell_1_1_2); 
TH1F* total_vcell_1_1_2 = dynamic_cast<TH1F*> (htotal_vcell_1_1_2); 

 TGraphAsymmErrors * effi_vcell_1_1_2 = new TGraphAsymmErrors ( match_vcell_1_1_2, total_vcell_1_1_2, "w" ); 

effi_vcell_1_1_2->SetTitle("");
effi_vcell_1_1_2->GetXaxis()->SetTitle("v position [pitch units]");
effi_vcell_1_1_2->GetYaxis()->SetTitle("average hit efficiency");
effi_vcell_1_1_2->GetXaxis()->SetRangeUser(0,768);
effi_vcell_1_1_2->GetYaxis()->SetRangeUser(0,1.0);
effi_vcell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effi_vcell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effi_vcell_1_1_2->GetYaxis()->SetTitleOffset(1.7);

effi_vcell_1_1_2->SetMarkerColor(kBlack);
effi_vcell_1_1_2->SetLineColor(kBlack);
effi_vcell_1_1_2->SetMarkerStyle(22);

ttbtrack->Draw(" vcell_fit_2_1_2 >> htotal_vcell_2_1_2(10,0,768)" , qc && roi_2 && region_u2);
ttbtrack->Draw(" vcell_fit_2_1_2 >> hmatch_vcell_2_1_2(10,0,768)" , "cluster_matched_2_1_2==0" && qc && roi_2 && region_u2 );

TH1F* match_vcell_2_1_2 = dynamic_cast<TH1F*> (hmatch_vcell_2_1_2); 
TH1F* total_vcell_2_1_2 = dynamic_cast<TH1F*> (htotal_vcell_2_1_2); 

TGraphAsymmErrors * effi_vcell_2_1_2 = new TGraphAsymmErrors ( match_vcell_2_1_2, total_vcell_2_1_2, "w" ); 

effi_vcell_2_1_2->SetTitle("");
effi_vcell_2_1_2->GetXaxis()->SetTitle("v position [pitch units]");
effi_vcell_2_1_2->GetYaxis()->SetTitle("average hit efficiency");
effi_vcell_2_1_2->GetXaxis()->SetRangeUser(0,768);
effi_vcell_2_1_2->GetYaxis()->SetRangeUser(0,1.0);//0.9,1
effi_vcell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effi_vcell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effi_vcell_2_1_2->GetYaxis()->SetTitleOffset(1.7);

effi_vcell_2_1_2->SetMarkerColor(kBlue);
effi_vcell_2_1_2->SetLineColor(kBlue);
effi_vcell_2_1_2->SetMarkerStyle(25);

effi_vcell_2_1_2->Draw(); 
effi_vcell_1_1_2->Draw("same"); 

TLegend* lc2 = new TLegend(0.6,0.2,0.85,0.4);
lc2->SetFillColor(kWhite); 
lc2->SetBorderSize(0);
lc2->SetTextSize(0.06);
lc2->AddEntry(effi_vcell_1_1_2,"PXD Layer 1","lep");
lc2->AddEntry(effi_vcell_2_1_2,"PXD Layer 2","lep");
lc2->Draw();
if (savePlots) c21->Print(TString::Format("%seff_v.pdf",plotBaseName.Data()));




TCanvas * c21a  = new TCanvas("c21a","c21a",600,400);
c21a->SetLeftMargin(0.2);
c21a->SetRightMargin(0.1);
c21a->SetTopMargin(0.1);
c21a->SetBottomMargin(0.16);

ttbtrack->Draw(" vcell_fit_1_1_2 >> htotalo_vcell_1_1_2(10,0,768)" , "otherpxd_cluster_matched_1_1_2 == 0" && qc && roi_1 && region_u1);
ttbtrack->Draw(" vcell_fit_1_1_2 >> hmatcho_vcell_1_1_2(10,0,768)" , "otherpxd_cluster_matched_1_1_2 == 0 && cluster_matched_1_1_2==0" && qc && roi_1 && region_u1);



TH1F* matcho_vcell_1_1_2 = dynamic_cast<TH1F*> (hmatcho_vcell_1_1_2); 
TH1F* totalo_vcell_1_1_2 = dynamic_cast<TH1F*> (htotalo_vcell_1_1_2); 

 TGraphAsymmErrors * effio_vcell_1_1_2 = new TGraphAsymmErrors ( matcho_vcell_1_1_2, totalo_vcell_1_1_2, "w" ); 

effio_vcell_1_1_2->SetTitle("");
effio_vcell_1_1_2->GetXaxis()->SetTitle("u position [pitch units]");
effio_vcell_1_1_2->GetYaxis()->SetTitle("avg. hit eff., hit on other pxd");
effio_vcell_1_1_2->GetXaxis()->SetRangeUser(0,768);
effio_vcell_1_1_2->GetYaxis()->SetRangeUser(0,1.0);
effio_vcell_1_1_2->GetXaxis()->SetTitleSize(0.06);
effio_vcell_1_1_2->GetYaxis()->SetTitleSize(0.06);
effio_vcell_1_1_2->GetXaxis()->SetLabelSize(0.06);
effio_vcell_1_1_2->GetYaxis()->SetLabelSize(0.06);
effio_vcell_1_1_2->GetYaxis()->SetTitleOffset(1.7);

effio_vcell_1_1_2->SetMarkerColor(kBlack);
effio_vcell_1_1_2->SetLineColor(kBlack);
effio_vcell_1_1_2->SetMarkerStyle(22);

ttbtrack->Draw(" vcell_fit_2_1_2 >> htotalo_vcell_2_1_2(10,0,768)" , "otherpxd_cluster_matched_2_1_2 == 0" && qc && roi_2 && region_u2);
ttbtrack->Draw(" vcell_fit_2_1_2 >> hmatcho_vcell_2_1_2(10,0,768)" , "otherpxd_cluster_matched_2_1_2 == 0 && cluster_matched_2_1_2==0" && qc && roi_2 && region_u2 );


TH1F* matcho_vcell_2_1_2 = dynamic_cast<TH1F*> (hmatcho_vcell_2_1_2); 
TH1F* totalo_vcell_2_1_2 = dynamic_cast<TH1F*> (htotalo_vcell_2_1_2); 

TGraphAsymmErrors * effio_vcell_2_1_2 = new TGraphAsymmErrors ( matcho_vcell_2_1_2, totalo_vcell_2_1_2, "w" ); 

effio_vcell_2_1_2->SetTitle("");
effio_vcell_2_1_2->GetXaxis()->SetTitle("v position [pitch units]");
effio_vcell_2_1_2->GetYaxis()->SetTitle("avg. hit eff., hit on other pxd");
effio_vcell_2_1_2->GetXaxis()->SetRangeUser(0,768);
effio_vcell_2_1_2->GetYaxis()->SetRangeUser(0,1.0);//0.9,1
effio_vcell_2_1_2->GetXaxis()->SetTitleSize(0.06);
effio_vcell_2_1_2->GetYaxis()->SetTitleSize(0.06);
effio_vcell_2_1_2->GetXaxis()->SetLabelSize(0.06);
effio_vcell_2_1_2->GetYaxis()->SetLabelSize(0.06);
effio_vcell_2_1_2->GetYaxis()->SetTitleOffset(1.7);

effio_vcell_2_1_2->SetMarkerColor(kBlue);
effio_vcell_2_1_2->SetLineColor(kBlue);
effio_vcell_2_1_2->SetMarkerStyle(25);

effio_vcell_2_1_2->Draw(); 
effio_vcell_1_1_2->Draw("same"); 

TLegend* lc2o = new TLegend(0.6,0.2,0.85,0.4);
lc2o->SetFillColor(kWhite); 
lc2o->SetBorderSize(0);
lc2o->SetTextSize(0.06);
lc2o->AddEntry(effi_vcell_1_1_2,"PXD Layer 1","lep");
lc2o->AddEntry(effi_vcell_2_1_2,"PXD Layer 2","lep");
lc2o->Draw();
if (savePlots) c21a->Print(TString::Format("%seff_v_other.pdf",plotBaseName.Data()));


TCanvas * c4a  = new TCanvas("c4a","c4a",600,400);
c4a->SetLeftMargin(0.2);
c4a->SetRightMargin(0.2);
c4a->SetTopMargin(0.1);
c4a->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_fit_1_1_2:ucell_fit_1_1_2 >> htotal_2dcell_1_1_2(50,0,250,150,0,768)" , qc && roi_1);
ttbtrack->Draw("vcell_fit_1_1_2:ucell_fit_1_1_2 >> hmatch_2dcell_1_1_2(50,0,250,150,0,768)" , "cluster_matched_1_1_2==0" && qc && roi_1);

TH2F* match_2dcell_1_1_2 = dynamic_cast<TH2F*> (hmatch_2dcell_1_1_2); 
TH2F* total_2dcell_1_1_2 = dynamic_cast<TH2F*> (htotal_2dcell_1_1_2); 

TEfficiency* eff_2dcell_1_1_2 = new TEfficiency(*match_2dcell_1_1_2,*total_2dcell_1_1_2);

eff_2dcell_1_1_2->SetTitle("PXD Layer 1 Efficiency;u position [pitch units];v position [pitch units]");
eff_2dcell_1_1_2->Draw("colz");
 c4a->Update();
 TH2F* effA = dynamic_cast<TH2F*> (eff_2dcell_1_1_2->GetPaintedHistogram());
 effA->SetStats( false );
 effA->GetZaxis()->SetTitle("number of clusters");
 effA->GetXaxis()->SetTitleSize(0.08);
 effA->GetYaxis()->SetTitleSize(0.08);
 effA->GetZaxis()->SetTitleSize(0.08);
 effA->GetXaxis()->SetLabelSize(0.08);
 effA->GetYaxis()->SetLabelSize(0.08);
 effA->GetZaxis()->SetLabelSize(0.08);
if (savePlots) c4a->Print(TString::Format("%seff_2d_1.pdf",plotBaseName.Data()));



TCanvas * c4b  = new TCanvas("c4b","c4b",600,400);
c4b->SetLeftMargin(0.2);
c4b->SetRightMargin(0.2);
c4b->SetTopMargin(0.1);
c4b->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_fit_1_1_2:ucell_fit_1_1_2 >> htotalo_2dcell_1_1_2(50,0,250,150,0,768)" , "otherpxd_cluster_matched_1_1_2 == 0" && qc && roi_1);
ttbtrack->Draw("vcell_fit_1_1_2:ucell_fit_1_1_2 >> hmatcho_2dcell_1_1_2(50,0,250,150,0,768)" , "otherpxd_cluster_matched_1_1_2 == 0 && cluster_matched_1_1_2==0" && qc && roi_1);

TH2F* matcho_2dcell_1_1_2 = dynamic_cast<TH2F*> (hmatcho_2dcell_1_1_2); 
TH2F* totalo_2dcell_1_1_2 = dynamic_cast<TH2F*> (htotalo_2dcell_1_1_2); 

TEfficiency* effo_2dcell_1_1_2 = new TEfficiency(*matcho_2dcell_1_1_2,*totalo_2dcell_1_1_2);

effo_2dcell_1_1_2->SetTitle("PXD Layer 1 Efficiency with hit required on Layer 2;u position [pitch units];v position [pitch units]");
effo_2dcell_1_1_2->Draw("colz");
 c4b->Update();
 TH2F* effB = dynamic_cast<TH2F*> (effo_2dcell_1_1_2->GetPaintedHistogram());
 effB->SetStats( false );
 effB->GetZaxis()->SetTitle("number of clusters");
 effB->GetXaxis()->SetTitleSize(0.08);
 effB->GetYaxis()->SetTitleSize(0.08);
 effB->GetZaxis()->SetTitleSize(0.08);
 effB->GetXaxis()->SetLabelSize(0.08);
 effB->GetYaxis()->SetLabelSize(0.08);
 effB->GetZaxis()->SetLabelSize(0.08);
if (savePlots) c4b->Print(TString::Format("%seff_2d_1_other.pdf",plotBaseName.Data()));




TCanvas * c4c  = new TCanvas("c4c","c4c",600,400);
c4c->SetLeftMargin(0.2);
c4c->SetRightMargin(0.2);
c4c->SetTopMargin(0.1);
c4c->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_fit_2_1_2:ucell_fit_2_1_2 >> htotal_2dcell_2_1_2(50,0,250,150,0,768)" , qc && roi_2);
ttbtrack->Draw("vcell_fit_2_1_2:ucell_fit_2_1_2 >> hmatch_2dcell_2_1_2(50,0,250,150,0,768)" , "cluster_matched_2_1_2==0" && qc && roi_2);

TH2F* match_2dcell_2_1_2 = dynamic_cast<TH2F*> (hmatch_2dcell_2_1_2); 
TH2F* total_2dcell_2_1_2 = dynamic_cast<TH2F*> (htotal_2dcell_2_1_2); 

TEfficiency* eff_2dcell_2_1_2 = new TEfficiency(*match_2dcell_2_1_2,*total_2dcell_2_1_2);

eff_2dcell_2_1_2->SetTitle("PXD Layer 2 Efficiency;u position [pitch units];v position [pitch units]");
eff_2dcell_2_1_2->Draw("colz");
 c4c->Update();
 TH2F* effC = dynamic_cast<TH2F*> (eff_2dcell_2_1_2->GetPaintedHistogram());
 effC->SetStats( false );
 effC->GetZaxis()->SetTitle("number of clusters");
 effC->GetXaxis()->SetTitleSize(0.08);
 effC->GetYaxis()->SetTitleSize(0.08);
 effC->GetZaxis()->SetTitleSize(0.08);
 effC->GetXaxis()->SetLabelSize(0.08);
 effC->GetYaxis()->SetLabelSize(0.08);
 effC->GetZaxis()->SetLabelSize(0.08);
if (savePlots) c4c->Print(TString::Format("%seff_2d_2.pdf",plotBaseName.Data()));



TCanvas * c4d  = new TCanvas("c4d","c4d",600,400);
c4d->SetLeftMargin(0.2);
c4d->SetRightMargin(0.2);
c4d->SetTopMargin(0.1);
c4d->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_fit_2_1_2:ucell_fit_2_1_2 >> htotalo_2dcell_2_1_2(50,0,250,150,0,768)" , "otherpxd_cluster_matched_2_1_2 == 0" && qc && roi_2);
ttbtrack->Draw("vcell_fit_2_1_2:ucell_fit_2_1_2 >> hmatcho_2dcell_2_1_2(50,0,250,150,0,768)" , "otherpxd_cluster_matched_2_1_2 == 0 && cluster_matched_2_1_2==0" && qc && roi_2);

TH2F* matcho_2dcell_2_1_2 = dynamic_cast<TH2F*> (hmatcho_2dcell_2_1_2); 
TH2F* totalo_2dcell_2_1_2 = dynamic_cast<TH2F*> (htotalo_2dcell_2_1_2); 

TEfficiency* effo_2dcell_2_1_2 = new TEfficiency(*matcho_2dcell_2_1_2,*totalo_2dcell_2_1_2);

effo_2dcell_2_1_2->SetTitle("PXD Layer 2 Efficiency with hit required on Layer 1;u position [pitch units];v position [pitch units]");
effo_2dcell_2_1_2->Draw("colz");
 c4d->Update();
 TH2F* effD = dynamic_cast<TH2F*> (effo_2dcell_2_1_2->GetPaintedHistogram());
 effD->SetStats( false );
 effD->GetZaxis()->SetTitle("number of clusters");
 effD->GetXaxis()->SetTitleSize(0.08);
 effD->GetYaxis()->SetTitleSize(0.08);
 effD->GetZaxis()->SetTitleSize(0.08);
 effD->GetXaxis()->SetLabelSize(0.08);
 effD->GetYaxis()->SetLabelSize(0.08);
 effD->GetZaxis()->SetLabelSize(0.08);
if (savePlots) c4d->Print(TString::Format("%seff_2d_2_other.pdf",plotBaseName.Data()));



TCanvas * c5a  = new TCanvas("c5a","c5a",600,400);
c5a->SetLeftMargin(0.2);
c5a->SetRightMargin(0.2);
c5a->SetTopMargin(0.1);
c5a->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_clus_1_1_2:ucell_clus_1_1_2 >> hhit_1_1_2(50,0,250,150,0,768)");

TH2F* hit_1_1_2 = dynamic_cast<TH2F*> (hhit_1_1_2); 

 hit_1_1_2->SetName("hit_1_1_2");
 hit_1_1_2->SetTitle("");
 hit_1_1_2->SetStats( false );
 hit_1_1_2->GetXaxis()->SetTitle("uCell Sensor 1.1.2");
 hit_1_1_2->GetYaxis()->SetTitle("vCell Sensor 1.1.2");
 hit_1_1_2->GetZaxis()->SetTitle("number of clusters");
 hit_1_1_2->GetXaxis()->SetTitleSize(0.08);
 hit_1_1_2->GetYaxis()->SetTitleSize(0.08);
 hit_1_1_2->GetZaxis()->SetTitleSize(0.08);
 hit_1_1_2->GetXaxis()->SetLabelSize(0.08);
 hit_1_1_2->GetYaxis()->SetLabelSize(0.08);
 hit_1_1_2->GetZaxis()->SetLabelSize(0.08);
 hit_1_1_2->Draw("colz");
 if (savePlots) c5a->Print(TString::Format("%shitmap_1.pdf",plotBaseName.Data()));




TCanvas * c5b  = new TCanvas("c5b","c5b",600,400);
c5b->SetLeftMargin(0.2);
c5b->SetRightMargin(0.2);
c5b->SetTopMargin(0.1);
c5b->SetBottomMargin(0.16);

ttbtrack->Draw("vcell_clus_2_1_2:ucell_clus_2_1_2 >> hhit_2_1_2(50,0,250,150,0,768)");
TH2F* hit_2_1_2 = dynamic_cast<TH2F*> (hhit_2_1_2); 

 hit_2_1_2->SetName("hit_2_1_2");
 hit_2_1_2->SetTitle("");
 hit_2_1_2->SetStats( false );
 hit_2_1_2->GetXaxis()->SetTitle("uCell Sensor 2.1.2");
 hit_2_1_2->GetYaxis()->SetTitle("vCell Sensor 2.1.2");
 hit_2_1_2->GetZaxis()->SetTitle("number of clusters");
 hit_2_1_2->GetXaxis()->SetTitleSize(0.08);
 hit_2_1_2->GetYaxis()->SetTitleSize(0.08);
 hit_2_1_2->GetZaxis()->SetTitleSize(0.08);
 hit_2_1_2->GetXaxis()->SetLabelSize(0.08);
 hit_2_1_2->GetYaxis()->SetLabelSize(0.08);
 hit_2_1_2->GetZaxis()->SetLabelSize(0.08);
 hit_2_1_2->Draw("colz");
 if (savePlots) c5b->Print(TString::Format("%shitmap_2.pdf",plotBaseName.Data()));

 //Change position here so it is correct for saved plots
 gStyle->SetStatY(0.97);
 // Set y-position (fraction of pad size)
 gStyle->SetStatX(0.6);
 // Set x-position (fraction of pad size)
 gStyle->SetStatW(0.35);
 // Set width of stat-box (fraction of pad size)
 gStyle->SetStatH(0.18);
 // Set height of stat-box (fraction of pad size)


TCanvas * c61a  = new TCanvas("c61a","c61a",600,400);
c61a->SetLeftMargin(0.2);
c61a->SetRightMargin(0.1);
c61a->SetTopMargin(0.1);
c61a->SetBottomMargin(0.16);

ttbtrack->Draw("roi_width_u_1_1_2 >>hsize(50,0,250)");
hsize->SetName("ROI Size"); 
hsize->SetTitle("");
hsize->GetXaxis()->SetTitle("uCell Sensor 1.1.2");
hsize->GetYaxis()->SetTitle("number of ROIs");
hsize->GetXaxis()->SetTitleSize(0.06);
hsize->GetYaxis()->SetTitleSize(0.06);
hsize->GetYaxis()->SetTitleOffset(1.5);
hsize->GetXaxis()->SetLabelSize(0.06);
hsize->GetYaxis()->SetLabelSize(0.06);
hsize->Draw("");
if (savePlots) c61a->Print(TString::Format("%sroi_size_u1.pdf",plotBaseName.Data()));


TCanvas * c61b  = new TCanvas("c61b","c61b",600,400);
c61b->SetLeftMargin(0.2);
c61b->SetRightMargin(0.1);
c61b->SetTopMargin(0.1);
c61b->SetBottomMargin(0.16);

ttbtrack->Draw("roi_width_v_1_1_2 >>hsize2(50,0,250)");
hsize2->SetName("ROI Size"); 
hsize2->SetTitle("");
hsize2->GetXaxis()->SetTitle("vCell Sensor 1.1.2");
hsize2->GetYaxis()->SetTitle("number of ROIs");
hsize2->GetXaxis()->SetTitleSize(0.06);
hsize2->GetYaxis()->SetTitleSize(0.06);
hsize2->GetYaxis()->SetTitleOffset(1.5);
hsize2->GetXaxis()->SetLabelSize(0.06);
hsize2->GetYaxis()->SetLabelSize(0.06);
hsize2->Draw("");
if (savePlots) c61b->Print(TString::Format("%sroi_size_v1.pdf",plotBaseName.Data()));

}
