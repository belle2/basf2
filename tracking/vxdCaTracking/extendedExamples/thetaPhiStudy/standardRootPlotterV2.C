#include <string>
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip> // setprecision
#include <sstream>
#include <stdio.h>

# include <TRandom.h>
# include <TH1D.h>
# include <TTree.h>
# include <TAxis.h>
# include <TFile.h>
# include <TCanvas.h>
# include <TMultiGraph.h>
# include <TLegend.h>
# include <TGraphAsymmErrors.h>

# include <TMath.h>
# include <TStyle.h>


using namespace std;

 void binLogX(TH1*h)
{

   const TAxis *axis = h->GetXaxis();
   int bins = axis->GetNbins();

   Axis_t from = axis->GetXmin();
   Axis_t to = axis->GetXmax();
   Axis_t width = (to - from) / bins;
   Axis_t *new_bins = new Axis_t[bins + 1];

   for (int i = 0; i <= bins; i++) {
     new_bins[i] = TMath::Power(10, from + i * width);

   }
   axis->Set(bins, new_bins);
   delete new_bins;
} 

/** test if branch exist in given ttree */
bool branchExists( const string& name, TTree& tree ) {
  TBranch* br = dynamic_cast< TBranch* >( tree.GetListOfBranches()->FindObject( name.c_str() ) );
  if (br) return true;
  cout << " branch " << name << " could not be found!" << endl;
  return false;
}


template<class HistType> 
void printRelevant(string what, HistType* hist, unsigned nDigits = 3) {
  double qResult[5];
  double qInput[5] = {0.05, 0.25, .5, 0.75, .95};
  double mean = hist->GetMean();
  double entries = hist->GetEntries();
  double integral = hist->Integral();
  
  hist->GetQuantiles(5, qResult, qInput);
  cout << what << "/relevantOutput " << what << " : total, mean, nEntries, quantile0.05, q0.25, median, q0.75, q0.95: " <<
  integral << std::fixed << std::setprecision(nDigits) <<
  "/" << mean <<
  "/" << std::fixed << std::setprecision(0) << entries << std::setprecision(nDigits) << "/" <<  qResult[0] << "/" <<  qResult[1] << "/" <<  qResult[2] << "/" <<  qResult[3] << "/" <<  qResult[4]<< endl;
}


void standardRootPlotterV2(string pTypeSet = "muons", double pMassSet = 105.7)
{
  // suppress popping up of canvases all the time
  gROOT->SetBatch(kTRUE);
  // open the file with simulated and reconstructed EvtGen particles
  gStyle->SetOptStat("neMRou"); // 
  /**     
   k :  kurtosis printed
   K :  kurtosis and kurtosis error *printed
   s :  skewness printed
   S :  skewness and skewness error printed
   i :  integral of bins printed
   I :  integral of bins with option "width" printed
   o :  number of overflows printed
   u :  number of underflows printed
   r :  rms printed
   R :  rms and rms error printed
   m :  mean value printed
   M :  mean value mean error values printed
   e :  number of entries printed
   n :  name of histogram is printed
   */

  bool doResiduals = false;
//   bool doPt = false;
  bool useBetaGamma = true;

  string fileName = "TFAnalizerResults.root";
  string revision = "r15345";
  string pType = pTypeSet; double pMass = pMassSet;
//   string pType = "muons"; double pMass = 105.7;
  // 	string pType = "pions"; double pMass = 139.6;
  // 	string pType = "kaons"; double pMass = 493.7;
  // 	string pType = "protons"; double pMass = 938.3;
  // 	string pType = "electrons"; double pMass = 0.511;
  // 	string pType = "evtGen"; double pMass = 0.555;
  cout << "pType/" << " relevantOutput /" << pTypeSet << "/"  << pMassSet << "/" << endl;

  
  stringstream describer;
  

  // open files for the validation histograms
  TFile* input = TFile::Open(fileName.c_str());
  TTree* tree = (TTree*) input->Get("m_treePtr"); // name of tree in root file
  TFile* output = TFile::Open("VXDTF_validate.root", "recreate");
  TCanvas* aCanvas = new TCanvas("c11","canvas1",1024,800);

  
  // determine detector type
  TH1I* caTrackLength = new TH1I("caTrackLength", "values of mcPts", 30, 0, 30); // pxRes = pointer in current scope, pX_residual = name of object in
  tree->Draw("CAreconstructedTrackLength >> caTrackLength");
  double meanTL = caTrackLength->GetMean();
  string detType = "SVD";
  if (meanTL > 6.) { detType = "VXD"; }

  describer << detType << revision << pType;
  
  
  // retrieving momentum information to recognize the loaded data automatically:
  TH1D* mcPt = new TH1D("mcPt", "values of mcPts", 1000, 0.02, 4.05); // captures the current pT distribution
  tree->Draw("TotalMCpTValues >> mcPt");
  TH1D* mcP = new TH1D("mcP", "values of mcPs", 1000, 0.02, 4.05); // captures the current momentum distribution
  tree->Draw("TotalMCMomValues >> mcP");
  cout  << describer.str() << " having momRMS: "<< mcP->GetRMS() << " and ptRMS: " << mcPt->GetRMS() << endl;
  
  if (pType != "evtGen") {
	if ( mcPt->GetRMS() < mcP->GetRMS()) { // pT was probably fixed
	  double meanpt = mcPt->GetMean()*1000.;
	  double rmspt = mcPt->GetRMS()*1000.;
	  cout << "pT/" << describer.str() << " relevantOutput pT /" << fixed << setprecision(1) << meanpt << "/MeV/" << rmspt << "/RMS" << endl;
	  describer <<  "pT" << fixed << setprecision(0) << meanpt << "MeV" << rmspt << "RMS" ;
	  
	  double meanMom = mcP->GetMean()*1000.;
	  double rmsMom = mcP->GetRMS()*1000.;
	  cout << "mom/" << describer.str() << " relevantOutput mom /" << fixed << setprecision(1) << meanMom << "/MeV/" << rmsMom << "/RMS" << endl;
	  meanMom /= pMass;
	  rmsMom /= pMass;
	  cout << "betaGamma/" << describer.str() << " relevantOutput betaGamma /" << fixed << setprecision(3) << meanMom << "/ /" << rmsMom << "/RMS" << endl;
	  describer << "betaGamma" << fixed << setprecision(2) << meanMom << "wRMS" << rmsMom ;
	} else {
	  double meanpt = mcPt->GetMean()*1000.;
	  double rmspt = mcPt->GetRMS()*1000.;
	  cout << "pT/" << describer.str() << " relevantOutput pT /" << fixed << setprecision(1) << meanpt << "/MeV/" << rmspt << "/RMS" << endl;
	  describer <<  "pT" << fixed << setprecision(0) << meanpt << "MeV" << rmspt << "RMS" ;
	  
	  double meanMom = mcP->GetMean()*1000.;
	  double rmsMom = mcP->GetRMS()*1000.;
	  cout << "mom/" << describer.str() << " relevantOutput mom /" << fixed << setprecision(1) << meanMom << "/MeV/" << rmsMom << "/RMS" << endl;
	  if (useBetaGamma) {
		meanMom /= pMass;
		rmsMom /= pMass;
		cout << "betaGamma/" << describer.str() << " relevantOutput betaGamma /" << fixed << setprecision(3) << meanMom << "/ /" << rmsMom << "/RMS" << endl;
		describer << "betaGamma" << fixed << setprecision(2) << meanMom << "wRMS" << rmsMom ;
	  } else {
		describer << "mom" << fixed << setprecision(0) << meanMom << "MeV" << rmsMom << "RMS" ;
	  }
	}
  }
  
  string nameShort = describer.str();
  
  

  if (doResiduals) {
	
	
	int nBinsResidual = 200;
	double minResidual = -0.1;
	double maxResidual = 0.1;

	
	// an example of a 1D histogram
	  // total TCs with QI > 0.7
	  TH1D* pxRes = new TH1D("pX_residual", "total residuals for momentum in x direction", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pxRes->GetXaxis()->SetTitle("p_x [GeV/c]");
	pxRes->GetYaxis()->SetTitle("# entries");
	tree->Draw("TotalPXresiduals>>pX_residual"); // unix pipe adapted for root tree
	pxRes->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the x-value of the momentum (px))"));
	pxRes->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pxRes->Draw();
	pxRes->Write();

	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("totalPxResiduals") +nameShort + string(".pdf")).c_str());
	  
	TH1D* pyRes = new TH1D("pY_residual", "total residuals for momentum in y direction", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pyRes->GetXaxis()->SetTitle("p_y [GeV/c]");
	pyRes->GetYaxis()->SetTitle("# entries");
	tree->Draw("TotalPYresiduals>>pY_residual"); // unix pipe adapted for root tree
	pyRes->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the y-value of the momentum (py))"));
	pyRes->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pyRes->Draw();
	pyRes->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("totalPyResiduals") +nameShort + string(".pdf")).c_str());

	TH1D* pzRes = new TH1D("pZ_residual", "total residuals for momentum in z direction", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pzRes->GetXaxis()->SetTitle("p_z [GeV/c]");
	pzRes->GetYaxis()->SetTitle("# entries");
	tree->Draw("TotalPZresiduals>>pZ_residual"); // unix pipe adapted for root tree
	pzRes->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the z-value of the momentum (pz))"));
	pzRes->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pzRes->Draw();
	pzRes->Write();
		  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("totalPzResiduals") +nameShort + string(".pdf")).c_str());
	
	TH1D* ptRes = new TH1D("pT_residual", "total residuals for momentum in pt", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	ptRes->GetXaxis()->SetTitle("p_T [GeV/c]");
	ptRes->GetYaxis()->SetTitle("# entries");
	tree->Draw("TotalCApTResiduals>>pT_residual"); // unix pipe adapted for root tree
	ptRes->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the t-value of the momentum (pz))"));
	ptRes->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	ptRes->Draw();
	ptRes->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("totalPtResiduals") +nameShort + string(".pdf")).c_str());

  //complete TCs
	TH1D* pxResC = new TH1D("pX_residualC", "residuals for momentum in x direction, complete TCs", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pxResC->GetXaxis()->SetTitle("p_x [GeV/c]");
	pxResC->GetYaxis()->SetTitle("# entries");
	tree->Draw("CompletePXresiduals>>pX_residualC"); // unix pipe adapted for root tree
	pxResC->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the x-value of the momentum (px))"));
	pxResC->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pxResC->Draw();
	pxResC->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("completePxResiduals") +nameShort + string(".pdf")).c_str());
	
	TH1D* pyResC = new TH1D("pY_residualC", "residuals for momentum in y direction, complete TCs", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pyResC->GetXaxis()->SetTitle("p_y [GeV/c]");
	pyResC->GetYaxis()->SetTitle("# entries");
	tree->Draw("CompletePYresiduals>>pY_residualC"); // unix pipe adapted for root tree
	pyResC->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the y-value of the momentum (py))"));
	pyResC->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pyResC->Draw();
	pyResC->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("completePyResiduals") +nameShort + string(".pdf")).c_str());

	TH1D* pzResC = new TH1D("pZ_residualC", "residuals for momentum in z direction, complete TCs", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	pzResC->GetXaxis()->SetTitle("p_z [GeV/c]");
	pzResC->GetYaxis()->SetTitle("# entries");
	tree->Draw("CompletePZresiduals>>pZ_residualC"); // unix pipe adapted for root tree
	pzResC->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the z-value of the momentum (pz))"));
	pzResC->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	pzResC->Draw();
	pzResC->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("completePzResiduals") +nameShort + string(".pdf")).c_str());
	  
	TH1D* ptResC = new TH1D("pT_residualC", "residuals for momentum in pt, complete TCs", nBinsResidual, minResidual, maxResidual); // pxRes = pointer in current scope, pX_residual = name of object in root-file
	ptResC->GetXaxis()->SetTitle("p_T [GeV/c]");
	ptResC->GetYaxis()->SetTitle("# entries");
	tree->Draw("CompleteCApTResiduals>>pT_residualC"); // unix pipe adapted for root tree
	ptResC->GetListOfFunctions()->Add(new TNamed("Description", "Residual (true - estimated) of momentum of the momentum-seed of the track candidates produced by the VXDTF. In this case it's the t-value of the momentum (pz))"));
	ptResC->GetListOfFunctions()->Add(new TNamed("Check", "The distribution is not gaussian since momenta of all TCs are considered. Outliers are mainly produced by comparing the wrong seeds (hit_true != hit_seed). The mean should be 0 and the total number of outliers small compared to the sample size (typical is around 1%)"));
	ptResC->Draw();
	ptResC->Write();
	  
	aCanvas->cd();
	aCanvas->Update();
	aCanvas->SaveAs( (string("completePtResiduals") +nameShort + string(".pdf")).c_str());
	
  }
  
  
  
  
  int nBins = 30;
  double xMinLog10 = log10( 0.035 );
  double xMaxLog10 = log10( 3.);
  stringstream effStream, ptStream;
  
  
  
  
  /// momEff momEff momEff momEff momEff
  
//   TCanvas* aCanvas = new TCanvas("c11","canvas1",1024,800);

  TMultiGraph* mgMom = new TMultiGraph();

  mgMom->SetTitle( "Efficiency vs momentum"); // collects plots about efficiency over total momentum

  TLegend* legendMom = new TLegend( 0.3, 0.11, 0.895, 0.27 );
  legendMom->SetFillStyle(-1); /// transparent legend!

  TH1D* histAll = new TH1D("AllMCMomValues","mctf-tracks;p_{T}",nBins, xMinLog10, xMaxLog10);
  binLogX( histAll);
  gPad->SetLogx(1);
  tree->Draw("TotalMCMomValues >> AllMCMomValues");

  TH1D* histFound = new TH1D("AllCAMomValues"," catf-tracks;p_{T}",nBins, xMinLog10, xMaxLog10);
  binLogX( histFound);
  tree->Draw("TotalCAMomValues >> AllCAMomValues");

  int totalRecomomtracks = histFound->Integral();
  int totalAllmomtracks = histAll->Integral();
  double momEff = 0.;
  if (totalAllmomtracks != 0) { momEff = double(totalRecomomtracks)/double(totalAllmomtracks); }
  effStream << std::fixed << std::setprecision( 3 ) << momEff;

  TGraphAsymmErrors* efficiencyHist = new TGraphAsymmErrors(histFound, histAll);
  efficiencyHist->Draw("AP");

  
  
  /// pTEff pTEff pTEff pTEff pTEff pTEff
  
  TH1D* histAllpT = new TH1D("AllMCpTValues","mctf-tracks;p_{T}",nBins, xMinLog10, xMaxLog10);
  binLogX( histAllpT);
  tree->Draw("TotalMCpTValues >> AllMCpTValues");

  TH1D* histFoundpT = new TH1D("AllCApTValues"," catf-tracks;p_{T}",nBins, xMinLog10, xMaxLog10);
  binLogX( histFoundpT);
  tree->Draw("TotalCApTValues >> AllCApTValues");

  int totalAllpTtracks = histAllpT->Integral();
  int totalRecopTtracks = histFoundpT->Integral();
  double pTEff = 0.;
  if (totalAllpTtracks != 0) { pTEff = double(totalRecopTtracks)/double(totalAllpTtracks); }
  ptStream << std::fixed << std::setprecision( 3 ) << pTEff;

  TGraphAsymmErrors* efficiencyHistpT = new TGraphAsymmErrors(histFoundpT, histAllpT);
  efficiencyHistpT->SetLineColor(kRed); //SetMarkerColor
  efficiencyHistpT->Draw("same");

  efficiencyHist->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHist->SetMarkerColor( 1 );
  efficiencyHist->SetMarkerStyle( 1 +2 );
  efficiencyHist->SetMarkerSize( 1 );
  efficiencyHist->SetLineColor( 1 );
  efficiencyHistpT->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistpT->SetMarkerColor( kRed );
  efficiencyHistpT->SetMarkerStyle( 1 +2 );
  efficiencyHistpT->SetMarkerSize( 1 );
  efficiencyHistpT->SetLineColor( kRed );

  aCanvas->Update();

  legendMom->AddEntry( efficiencyHist, ((string("Efficiency p ")+ effStream.str() +nameShort).c_str()));
  mgMom->Add( efficiencyHist);
  legendMom->AddEntry( efficiencyHistpT, ((string("Efficiency pT ")+ ptStream.str() +nameShort).c_str()));
  mgMom->Add( efficiencyHistpT);


  aCanvas->cd();
  gPad->SetLogx(1);
  mgMom->Draw("AP");
  aCanvas->SetGridx(1);
  aCanvas->SetGridy(1);
  mgMom->GetYaxis()->SetRangeUser(0.,1.);
  mgMom->GetXaxis()->SetTitle( "momentum [GeV/c]" );
  legendMom->Draw("same");
  aCanvas->Update();
  aCanvas->SaveAs( (string("totalMomentumAndPt") +nameShort + string(".pdf")).c_str() );

  
  
  

  

  
  /// THETA THETA THETA THETA THETA THETA
  TCanvas* tetCanvas = new TCanvas("ctet","canvastet",1024,800);


  double tetMin = 15.;
  double tetMax = 155.;
  gPad->SetLogx(0);
  int nBinsTet = (int(tetMax) - int(tetMin))/5;

  TH1D* histAllTet = new TH1D("AllMCThetaValues","mctf-tracks;#theta",nBinsTet, tetMin, tetMax);
  tree->Draw("TotalMCThetaValues >> AllMCThetaValues");

  int totalAllmomtracksTet = histAllTet->Integral();

  TH1D* histFoundTet = new TH1D("AllCAThetaValues"," catf-tracks;#theta",nBinsTet, tetMin, tetMax);
  tree->Draw("TotalCAThetaValues >> AllCAThetaValues");

  int totalRecomomtracksTet = histFoundTet->Integral();
  double momEffTet = 0.;
  if (totalAllmomtracksTet != 0) { momEffTet = double(totalRecomomtracksTet)/double(totalAllmomtracksTet); }
  stringstream tetStream;
  tetStream << std::fixed << std::setprecision( 3 ) << momEffTet;

  
  TGraphAsymmErrors* efficiencyHistTet = new TGraphAsymmErrors(histFoundTet, histAllTet);
  efficiencyHistTet->Draw("AP");
  efficiencyHistTet->SetTitle( "Efficiency of #theta" );
  efficiencyHistTet->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistTet->GetXaxis()->SetTitle( "#theta [degrees]" );
  efficiencyHistTet->GetXaxis()->SetRangeUser(tetMin,tetMax);
  efficiencyHistTet->SetMarkerColor( 1 );
  efficiencyHistTet->SetMarkerStyle( 1 +2 );
  efficiencyHistTet->SetMarkerSize( 1 );
  efficiencyHistTet->SetLineColor( 1 );


  // add legend
  TLegend* legendTet = new TLegend( 0.3, 0.11, 0.895, 0.2 );
  legendTet->SetFillStyle(-1); /// transparent legend!
  legendTet->AddEntry( efficiencyHistTet, ((string("Efficiency #theta ")+ tetStream.str() +nameShort).c_str()));
  legendTet->Draw("same");

  
  tetCanvas->SetGridx(1);
  tetCanvas->SetGridy(1);
  tetCanvas->SaveAs( (string("totalTheta") +nameShort + string(".pdf")).c_str() );








  /// PHI PHI PHI PHI PHI PHI
  
  // TCanvas brauche ich eigentlich nur einmal pro script, wird immer mit draw upgedated
  TCanvas* globalCanvas = new TCanvas("myCanvas","mycanvas",1024,800); 

/** TODO Was ist hier zu tun? TODO 
 * 
 * also, das ist hier mittlerweile ein fettes Kuddelmuddel!
 * sorge dafür, dass es für jeden Plot-typ eine Function existiert, welche mir diesen Plot erzeugen kann
 * (muss also nur noch passende Parameter übergeben und fertig is die Geschichte)
 * 
 */

  double phiMin = 0.;
  double phiMax = 360.;
  int nBinsPhi = (int(phiMax) - int(phiMin))/5;
  gPad->SetLogx(0);

  TH1D* histAllPhi = new TH1D("AllMCPhiValues","mctf-tracks;#Phi",nBinsPhi, phiMin, phiMax);
  tree->Draw("TotalMCPhiValues >> AllMCPhiValues");
  
  int totalAllmomtracksPhi = histAllPhi->Integral();

  TH1D* histFoundPhi = new TH1D("AllCAPhiValues"," catf-tracks;#Phi",nBinsPhi, phiMin, phiMax);
  tree->Draw("TotalCAPhiValues >> AllCAPhiValues");

  
  TGraphAsymmErrors* efficiencyHistPhi = new TGraphAsymmErrors(histFoundPhi, histAllPhi);
  efficiencyHistPhi->Draw("AP");
  efficiencyHistPhi->SetTitle( "Efficiency of #phi" );
  efficiencyHistPhi->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistPhi->GetXaxis()->SetTitle( "#phi [degrees]" );
  efficiencyHistPhi->GetXaxis()->SetRangeUser(phiMin-1,phiMax+1);
  efficiencyHistPhi->SetMarkerColor( 1 );
  efficiencyHistPhi->SetMarkerStyle( 1 +2 );
  efficiencyHistPhi->SetMarkerSize( 1 );
  efficiencyHistPhi->SetLineColor( 1 );
  
// calc efficiency in total:  
  int totalRecomomtracksPhi = histFoundPhi->Integral();
  double momEffPhi = 0.;
  if (totalAllmomtracksPhi != 0) { momEffPhi = double(totalRecomomtracksPhi)/double(totalAllmomtracksPhi); }
  stringstream phiStream;
  phiStream << std::fixed << std::setprecision( 3 ) << momEffPhi;

// add legend
  TLegend* legendPhi = new TLegend( 0.3, 0.11, 0.895, 0.2 );
  legendPhi->SetFillStyle(-1); /// transparent legend!
  legendPhi->AddEntry( efficiencyHistPhi, ((string("Efficiency #phi ")+ phiStream.str() +nameShort).c_str()));
  legendPhi->Draw("same");
  
  globalCanvas->SetGridx(1);
  globalCanvas->SetGridy(1);
  globalCanvas->SaveAs( (string("efficiencyPhi") +nameShort + string(".pdf")).c_str() );


  

  
  /// someOutput:
  cout << "sampleSize/" << "relevantOutput " << " sampleSize mc for " << nameShort << ": /" << totalRecomomtracksPhi << "/" << endl;
  cout << "efficiency/" << "relevantOutput " << "for mean Theta " << histAllTet->GetMean() << " efficiencyPhi " << nameShort << " /" << std::fixed << std::setprecision( 3 )  << momEffPhi*100. << "/" << endl;

  string lostUClusters = "LostUClusters";
  if (branchExists(lostUClusters, *tree) ) {
	TH1I* lostUClustersT = new TH1I((lostUClusters + "1").c_str(), "lost clusters in U", 10, 0, 10);
	tree->Draw((lostUClusters + " >> " + lostUClusters + "1").c_str());
	printRelevant(lostUClusters, lostUClustersT);
  }
  
  string lostVClusters = "LostVClusters";
  if (branchExists(lostVClusters, *tree) ) {
	TH1I* lostVClustersT = new TH1I((lostVClusters + "1").c_str(), "lost clusters in V", 10, 0, 10);
	tree->Draw((lostVClusters + " >> " + lostVClusters + "1").c_str());
	printRelevant(lostVClusters, lostVClustersT);
  }

  string totalMcUClusters = "TotalMcUClusters";
  if (branchExists(totalMcUClusters, *tree) ) {
	TH1I* totalMcUClustersT = new TH1I((totalMcUClusters + "1").c_str(), "total clusters in U", 10, 0, 10);
	tree->Draw((totalMcUClusters + " >> " + totalMcUClusters + "1").c_str());
	printRelevant(totalMcUClusters, totalMcUClustersT);
  }
  
  string totalMcVClusters = "TotalMcVClusters";
  if (branchExists(totalMcVClusters, *tree) ) {
	TH1I* totalMcVClustersT = new TH1I((totalMcVClusters + "1").c_str(), "total clusters in V", 10, 0, 10);
	tree->Draw((totalMcVClusters + " >> " + totalMcVClusters + "1").c_str());
	printRelevant(totalMcVClusters, totalMcVClustersT);
  }

  
  
  string lostUClusterEDep = "LostUClusterEDep";
  if (branchExists(lostUClusterEDep, *tree) ) {
	TH1I* lostUClusterEDepT = new TH1I((lostUClusterEDep + "1").c_str(), "lost eDep in U", 250, 1, 25000);
	tree->Draw((lostUClusterEDep + " >> " + lostUClusterEDep + "1").c_str());
	printRelevant(lostUClusterEDep, lostUClusterEDepT, unsigned(8));
  }
  
  string lostVClusterEDep = "LostVClusterEDep";
  if (branchExists(lostVClusterEDep, *tree) ) {
	TH1I* lostVClusterEDepT = new TH1I((lostVClusterEDep + "1").c_str(), "lost eDep in U", 250, 1, 25000);
	tree->Draw((lostVClusterEDep + " >> " + lostVClusterEDep + "1").c_str());
	printRelevant(lostVClusterEDep, lostVClusterEDepT, unsigned(8));
  }
  
  string totalMcUClusterEDep = "TotalMcUClusterEDep";
  if (branchExists(totalMcUClusterEDep, *tree) ) {
	TH1I* totalMcUClusterEDepT = new TH1I((totalMcUClusterEDep + "1").c_str(), "total eDep in U", 250, 1, 25000);
	tree->Draw((totalMcUClusterEDep + " >> " + totalMcUClusterEDep + "1").c_str());
	printRelevant(totalMcUClusterEDep, totalMcUClusterEDepT, unsigned(8));
  }
  
  string totalMcVClusterEDep = "TotalMcVClusterEDep";
  if (branchExists(totalMcVClusterEDep, *tree) ) {
	TH1I* totalMcVClusterEDepT = new TH1I((totalMcVClusterEDep + "1").c_str(), "total eDep in U", 250, 1, 25000);
	tree->Draw((totalMcVClusterEDep + " >> " + totalMcVClusterEDep + "1").c_str());
	printRelevant(totalMcVClusterEDep, totalMcVClusterEDepT, unsigned(8));
  }


  
  
  
  /// Efficiency vs IP-distance in 3D 3D 3D 3D 3D
  
  double distMin = 0.;
  double distMax = 2.;
  int nBinsdist = 50;
  double xyModifier = 1./2.5;
  gPad->SetLogx(0);
  
  TH1D* histAllDist2IP3D = new TH1D("AllMCVertex2IP3DValues","mctf-tracks;dist to IP",nBinsdist, distMin, distMax);
  tree->Draw("TotalMCVertex2IP3DValues >> AllMCVertex2IP3DValues");
  
  histAllDist2IP3D->Draw();  
  
  int totalAllmomtracksDist2IP3D = histAllDist2IP3D->Integral();
  
  TH1D* histFoundDist2IP3D = new TH1D("AllCAVertex2IP3DValues"," catf-tracks;dist to IP",nBinsdist, distMin, distMax);
  tree->Draw("TotalCAVertex2IP3DValues >> AllCAVertex2IP3DValues");
  
  histFoundDist2IP3D->Draw();
  
  
  
  TGraphAsymmErrors* efficiencyHistDist2IP3D = new TGraphAsymmErrors(histFoundDist2IP3D, histAllDist2IP3D);
  efficiencyHistDist2IP3D->Draw("AP");
  efficiencyHistDist2IP3D->SetTitle( "Efficiency of dist to IP in 3D" );
  efficiencyHistDist2IP3D->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistDist2IP3D->GetXaxis()->SetTitle( "dist to IP in cm" );
  efficiencyHistDist2IP3D->GetXaxis()->SetRangeUser(distMin-1,distMax+1);
  efficiencyHistDist2IP3D->SetMarkerColor( 1 );
  efficiencyHistDist2IP3D->SetMarkerStyle( 1 +2 );
  efficiencyHistDist2IP3D->SetMarkerSize( 1 );
  efficiencyHistDist2IP3D->SetLineColor( 1 );
  
  // calc efficiency in total:  
  int totalRecomomtracksDist2IP3D= histFoundDist2IP3D->Integral();
  double momEffDist2IP3D = 0.;
  if (totalAllmomtracksDist2IP3D != 0) { momEffDist2IP3D = double(totalRecomomtracksDist2IP3D)/double(totalAllmomtracksDist2IP3D); }
  stringstream streamDist2IP3D;
  streamDist2IP3D << std::fixed << std::setprecision( 3 ) << momEffDist2IP3D;
  
  // add legend
  TLegend* legendDist2IP3D = new TLegend( 0.3, 0.11, 0.895, 0.2 );
  legendPhi->SetFillStyle(-1); /// transparent legend!
  legendPhi->AddEntry( efficiencyHistDist2IP3D, ((string("Efficiency dist2IPin3D")+ phiStream.str() +nameShort).c_str()));
  legendPhi->Draw("same");
  
  globalCanvas->SetGridx(1);
  globalCanvas->SetGridy(1);
  globalCanvas->SaveAs( (string("efficiencyDist2IP3D") +nameShort + string(".pdf")).c_str() );







  /// Efficiency vs IP-distance in XY XY XY XY XY XY
  
  gPad->SetLogx(0);
  
  TH1D* histAllDist2IPXY = new TH1D("AllMCVertex2IPXYValues","mctf-tracks;dist to IP",nBinsdist, distMin, distMax*xyModifier);
  tree->Draw("TotalMCVertex2IPXYValues >> AllMCVertex2IPXYValues");
  
  histAllDist2IPXY->Draw();  
  
  int totalAllmomtracksDist2IPXY = histAllDist2IPXY->Integral();
  
  TH1D* histFoundDist2IPXY = new TH1D("AllCAVertex2IPXYValues"," catf-tracks;dist to IP",nBinsdist, distMin, distMax*xyModifier);
  tree->Draw("TotalCAVertex2IPXYValues >> AllCAVertex2IPXYValues");
  
  histFoundDist2IPXY->Draw();
  
  
  
  TGraphAsymmErrors* efficiencyHistDist2IPXY = new TGraphAsymmErrors(histFoundDist2IPXY, histAllDist2IPXY);
  efficiencyHistDist2IPXY->Draw("AP");
  efficiencyHistDist2IPXY->SetTitle( "Efficiency of dist to IP in XY" );
  efficiencyHistDist2IPXY->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistDist2IPXY->GetXaxis()->SetTitle( "dist to IP in cm" );
  efficiencyHistDist2IPXY->GetXaxis()->SetRangeUser(distMin-1,distMax*0.2 + 1);
  efficiencyHistDist2IPXY->SetMarkerColor( 1 );
  efficiencyHistDist2IPXY->SetMarkerStyle( 1 +2 );
  efficiencyHistDist2IPXY->SetMarkerSize( 1 );
  efficiencyHistDist2IPXY->SetLineColor( 1 );
  
  // calc efficiency in total:  
  int totalRecomomtracksDist2IPXY = histFoundDist2IPXY->Integral();
  double momEffDist2IPXY = 0.;
  if (totalAllmomtracksDist2IPXY != 0) { momEffDist2IPXY = double(totalRecomomtracksDist2IPXY)/double(totalAllmomtracksDist2IPXY); }
  stringstream streamDist2IPXY;
  streamDist2IPXY << std::fixed << std::setprecision( 3 ) << momEffDist2IPXY;
  
  // add legend
  TLegend* legendDist2IPXY = new TLegend( 0.3, 0.11, 0.895, 0.2 );
  legendDist2IPXY->SetFillStyle(-1); /// transparent legend!
  legendDist2IPXY->AddEntry( efficiencyHistDist2IPXY, ((string("Efficiency dist2IPinXY")+ streamDist2IPXY.str() +nameShort).c_str()));
  legendDist2IPXY->Draw("same");
  
  globalCanvas->SetGridx(1);
  globalCanvas->SetGridy(1);
  globalCanvas->SaveAs( (string("efficiencyDist2IPXY") +nameShort + string(".pdf")).c_str() );







  /// Efficiency vs IP-distance in Z Z Z Z Z Z
  
  gPad->SetLogx(0);
  
  TH1D* histAllDist2IPZ = new TH1D("AllMCVertex2IPZValues","mctf-tracks;dist to IP",nBinsdist, distMin, distMax);
  tree->Draw("TotalMCVertex2IPZValues >> AllMCVertex2IPZValues");
  
  histAllDist2IPZ->Draw();  
  
  int totalAllmomtracksDist2IPZ = histAllDist2IPZ->Integral();
  
  TH1D* histFoundDist2IPZ = new TH1D("AllCAVertex2IPZValues"," catf-tracks;dist to IP",nBinsdist, distMin, distMax);
  tree->Draw("TotalCAVertex2IPZValues >> AllCAVertex2IPZValues");
  
  histFoundDist2IPZ->Draw();
  
  
  
  TGraphAsymmErrors* efficiencyHistDist2IPZ = new TGraphAsymmErrors(histFoundDist2IPZ, histAllDist2IPZ);
  efficiencyHistDist2IPZ->Draw("AP");
  efficiencyHistDist2IPZ->SetTitle( "Efficiency of dist to IP in Z" );
  efficiencyHistDist2IPZ->GetYaxis()->SetRangeUser(0.,1.);
  efficiencyHistDist2IPZ->GetXaxis()->SetTitle( "dist to IP in cm" );
  efficiencyHistDist2IPZ->GetXaxis()->SetRangeUser(distMin-1,distMax+1);
  efficiencyHistDist2IPZ->SetMarkerColor( 1 );
  efficiencyHistDist2IPZ->SetMarkerStyle( 1 +2 );
  efficiencyHistDist2IPZ->SetMarkerSize( 1 );
  efficiencyHistDist2IPZ->SetLineColor( 1 );
  
  // calc efficiency in total:  
  int totalRecomomtracksDist2IPZ = histFoundDist2IPZ->Integral();
  double momEffDist2IPZ = 0.;
  if (totalAllmomtracksDist2IPZ != 0) { momEffDist2IPZ = double(totalRecomomtracksDist2IPZ)/double(totalAllmomtracksDist2IPZ); }
  stringstream streamDist2IPZ;
  streamDist2IPZ << std::fixed << std::setprecision( 3 ) << momEffDist2IPZ;
  
  // add legend
  TLegend* legendDist2IPZ = new TLegend( 0.3, 0.11, 0.895, 0.2 );
  legendDist2IPZ->SetFillStyle(-1); /// transparent legend!
  legendDist2IPZ->AddEntry( efficiencyHistDist2IPZ, ((string("Efficiency dist2IPinZ")+ streamDist2IPZ.str() +nameShort).c_str()));
  legendDist2IPZ->Draw("same");
  
  globalCanvas->SetGridx(1);
  globalCanvas->SetGridy(1);
  globalCanvas->SaveAs( (string("efficiencyDist2IPZ") +nameShort + string(".pdf")).c_str() );
  
  /*
//   efficiencyHistPhi->Draw("AP");

//   phiCanvas->SetGridx(1);
//   phiCanvas->SetGridy(1);
//   efficiencyHistPhi->SetTitle( "Efficiency of Phi" );
//   efficiencyHistPhi->GetYaxis()->SetRangeUser(0.,1.);
// //   efficiencyHistPhi->GetXaxis()->SetRangeUser(phiMin,phiMax);
//   efficiencyHistPhi->GetXaxis()->SetTitle( "\Phi [°]" );
//   efficiencyHistPhi->SetMarkerColor( 1 );
//   efficiencyHistPhi->SetMarkerStyle( 1 +2 );
//   efficiencyHistPhi->SetMarkerSize( 1 );
//   efficiencyHistPhi->SetLineColor( 1 );

  phiCanvas->Update();

  legendPhi->AddEntry( efficiencyHistPhi, ((string("Efficiency Phi ")+ phiStream.str() +nameShort).c_str()));
  mgPhi->Add( efficiencyHistPhi);


  phiCanvas->cd();
//   gPad->SetLogx(0);
  mgPhi->Draw("AP");
  phiCanvas->SetGridx(1);
  phiCanvas->SetGridy(1);
  mgPhi->GetYaxis()->SetRangeUser(0.,1.);
  mgPhi->GetXaxis()->SetTitle( "#Phi [degrees]" );
  mgPhi->GetXaxis()->SetRangeUser(phiMin,phiMax);
  legendPhi->Draw("same");
  phiCanvas->Update();
  phiCanvas->SaveAs( (string("totalPhi") +nameShort + string(".pdf")).c_str() );
  */

  delete output;
}



struct HistSetup {
  double min; /**< min value for range */
  double max; /**< max value for range */
  double nBins; /**< total number of bins you want to have */
  std::string nameOfStuff; /**< store here the identifying name of the Histogram you want to be printed */
  std::string additionalInfo; /**< some extra bla to identify the total setup this histogram is associated with */
  TCanvas* aCanvas; /**< just to have a canvas to draw on */
  TTree* tree; /**< tree carrying data to be imported */
  
};
  
  
class LinearEfficiencyPlot {
  
public:
  /** constructor.
   * 
   * 
   * */
  LinearEfficiencyPlot(std::string refName, std::string tsName, HistSetup mySetup) : setup(mySetup) {
	reference.first = string("reference_") + setup.nameOfStuff;
	reference.second = new TH1D(reference.first.c_str(),(string("referenceEntries;") + setup.nameOfStuff).c_str(),setup.nBins, setup.min, setup.max);
	setup.tree->Draw((refName + " >> " + reference.first).c_str());
	
	testSample.first = string("testSample_") + setup.nameOfStuff;
	testSample.second = new TH1D(testSample.first.c_str(),(string("testSampleEntries;") + setup.nameOfStuff).c_str(),setup.nBins, setup.min, setup.max);
	setup.tree->Draw((tsName + " >> " + testSample.first).c_str());
  }
  
  /** destructor for fine cleanup */
  ~LinearEfficiencyPlot() {
	delete reference.second;
	delete testSample.second;
	delete efficiencyPlot.second;
  }
  
  /** export reference and testSample */
  void ExportSinglePlots() {
	PrintHist(reference);
	PrintHist(testSample);
  }
  
  
protected:
  /** general printer */
  void PrintHist(std::pair<std::string, TH1D*>& hist) {
	hist.second->Draw();
	setup.aCanvas->SaveAs(( hist.first + setup.additionalInfo + string(".pdf")).c_str() );
  }
  
  std::pair<std::string, TH1D*> reference; /**< .first stores reference name and .second stores reference plot (has to have at least as many entries per bin as test testSample */
  std::pair<std::string, TH1D*> testSample; /**< .first stores sample name and .second stores sample to be compared with reference */
  std::pair<std::string, TGraphAsymmErrors*> efficiencyPlot; /**< .first stores efficiency name and .second stores calculated efficiency */
  HistSetup setup; /**< stores relevant parameters like nBins, range, name and general stuff like the tree and canvas */
};


# ifndef __CINT__
int main()
{
  standardRootPlotterV2();
}
# endif