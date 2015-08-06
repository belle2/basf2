/*
<header>
<input>SVDValidationTTree.root</input>
<input>SVDValidationTTreeDigit.root</input>
<input>SVDValidationTTreeSimhit.root</input>
<input>SVDValidationTTreeEfficiency.root</input>
<description>
    This ROOT macro is used for the SVD validation. It creates several
    histograms, divided by variable,layer number, strip direction (U, V), 
    type of sensor (barrel or slanted, Layer3 type is called barrel too for simplicity's sake)
    and saves them to a ROOT file.
</description>
<contact>G.Caria, gcaria@student.unimelb.edu.au</contact>
</header>
*/
#include <TStyle.h>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TEfficiency.h"

// Function that converts layer number into layer index used in for loops
int layerToIndex(int layer){
  int layermin = 3;
  int layerIndex;
  layerIndex = layer - layermin;	      
  return layerIndex;
}

void SVDValidation()
{
//-------------------------------------------------------------
  //Parameters for all plots
  int bins = 100;
  const int nAngles = 2;
  const int nLayers = 4; 
  const int nTypes = 2;
  const int nSides = 2;
  int layerMin = 3;
  int layerMax = 6;
  int canvasWidth = 900;
  int canvasHeight = 700;

  const char *Side[] = {"U","V"};
  const char *Type[] = {"Slanted","Barrel"};
  
  float minTheta = 10;
  float maxTheta = 160;
  float minPhi = -20;
  float maxPhi = 360;

  // Function declaration
  int layerToIndex(int layer);

//-------------------------------------------------------------
  // open the files with simulated and reconstructed events data
  TFile* input = TFile::Open("../SVDValidationTTree.root");
  TFile* inputDigit = TFile::Open("../SVDValidationTTreeDigit.root");
  TFile* inputSimhit = TFile::Open("../SVDValidationTTreeSimhit.root");
  TFile* inputEff = TFile::Open("../SVDValidationTTreeEfficiency.root");

  // open the output file for the validation histograms
  TFile* output = TFile::Open("SVDValidationPlots.root", "recreate");

//------------------------------------------------------------------  
  // loads ttrees
  TTree* tree = (TTree*) input->Get("tree");
  TTree* treeDigit = (TTree*) inputDigit->Get("tree");
  TTree* treeSimhit = (TTree*) inputSimhit->Get("tree");
  TTree* treeUEff = (TTree*) inputEff->Get("treeU");
  TTree* treeVEff = (TTree*) inputEff->Get("treeV");

//------------------------------------------------------------------  
  // Histograms declarations
  TH1F *hValidEta[nLayers][nTypes][nSides]; 
  TH1F *hValidClusterSize[nLayers][nTypes][nSides]; 
  TH1F *hValidPull[nLayers][nTypes][nSides]; 
  TH1F *hValidClusterCharge[nLayers][nTypes][nSides];
  TH1F *hValidSeedCharge[nLayers][nTypes][nSides];
  TH1F *hValidSimhitdEdX[nLayers][nTypes];
  TH2F *hValidTrueHitDepChargevsEnLost[nLayers][nTypes];
  TH2F *hValidEffTotal[nLayers][nTypes][nSides];
  TH2F *hValidEffPassed[nLayers][nTypes][nSides];
  TH1F *hValidDigitSignalToNoise[nLayers][nTypes][nSides];

//----------------------------------------------------------------

//********************************PLOTS********************************
  
//---------------------------------------------------------------------
// Clusters: Eta plots 
   
  int etaBins = 50;
  float minEta = 0;
  float maxEta = 1;

  for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
      int layerIndex = layerToIndex(layer);
      // build canvas name
      TString canvasName(Form("cValidEtaDistributionLayer%d", layer));
      TString canvasTitle(Form("Eta Distributions, Layer %d", layer));
      TCanvas *cValidEtaDist;
      cValidEtaDist = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
      cValidEtaDist->Divide(2,2);
      cValidEtaDist->Draw();
      int k=0;	    
      for (int m=1; m>=0; m--) { // loop over types
          for (int i=0; i<=1; i++) { //loop over sides
              TString histoTitle(Form("Validation: Eta, layer %d, %s, %s side", layer,Type[m],Side[i]));
              TString histoName(Form("hValidEta%s_Layer%d_%s",Side[i], layer,Type[m]));
              k=k+1;
              cValidEtaDist->cd(k);
              if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
              // build histogram
              hValidEta[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minEta,maxEta);
              // set axes labels
              hValidEta[layerIndex][m][i]->GetXaxis()->SetTitle("Eta");
              hValidEta[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
              // draw histograms
              TString expr(Form("cluster_eta>>%s",histoName.Data()));
              TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 

      	      tree->Draw(expr,cond);

              hValidEta[layerIndex][m][i]->GetListOfFunctions()->Add(new
                      TNamed("Description", "Validation: Eta = (cluster_pos %% pitch / pitch) distributions"));
              hValidEta[layerIndex][m][i]->GetListOfFunctions()->Add(new
                      TNamed("Check", "In ideal world should be a uniform distribution"));
              hValidEta[layerIndex][m][i]->Write(histoName);
          } // sides loop ends
      } // types loop ends
         cValidEtaDist->Write(canvasName);
  } // layers loop ends

//---------------------------------------------------------------------
// Clusters: Pull plots 
    
  int minPull = -10;
  int maxPull = 10;
   
  for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
      int layerIndex = layerToIndex(layer);
      // build canvas name
      TString canvasName(Form("cValidPullDistributionLayer%d", layer));
      TString canvasTitle(Form("Pull Distributions, Layer %d", layer));
      TCanvas *cValidPullDist;
      cValidPullDist = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
      cValidPullDist->Divide(2,2);
      cValidPullDist->Draw();
      int k=0;	    
      for (int m=1; m>=0; m--) { // loop over types
          for (int i=0; i<=1; i++) { //loop over sides
              TString histoTitle(Form("Validation: Pull, layer %d, %s, %s side", layer,Type[m],Side[i]));
              TString histoName(Form("hValidPull%s_Layer%d_%s",Side[i], layer,Type[m]));
              k=k+1;
              cValidPullDist->cd(k);
              if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
              // build histogram
              hValidPull[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minPull,maxPull);
              // set axes labels
              hValidPull[layerIndex][m][i]->GetXaxis()->SetTitle("Pull");
              hValidPull[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
              // draw histograms
              TString expr(Form("cluster_pull>>%s",histoName.Data()));
              TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 

      	      tree->Draw(expr,cond);

              hValidPull[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Pull (clusterPos - truehitPos/clusterPosSignma) distributions"));
              hValidPull[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Check", "Should be centered at 0 with RMS less than 2.0"));
              hValidPull[layerIndex][m][i]->Write(histoName);
          } // sides loop ends
      } // types loop ends
         cValidPullDist->Write(canvasName);
  } // layers loop ends

//----------------------------------------------------------------------
// Clusters: Cluster size plots 
    
  int minSize = 0;
  int maxSize = 10;
 
  for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValidSizeDistributionLayer%d", layer));
    TString canvasTitle(Form("Cluster Size Distributions, Layer %d", layer));
    TCanvas *cValidSizeDist;
    cValidSizeDist = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidSizeDist->Divide(2,2);
    cValidSizeDist->Draw();
    int k=0;        
    for (int  m=1; m>=0; m--) { // loop over types
        for (int i=0; i<=1; i++) { // loop over sides
	        TString histoTitle(Form("Validation: Cluster size, Layer %d, %s, %s side", layer,Type[m],Side[i]));
            TString histoName(Form("hValidClusterSize%s_Layer%d_%s",Side[i], layer,Type[m]));
            k=k+1;            
            cValidSizeDist->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
            // build histogram
            hValidClusterSize[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minSize,maxSize);
            // set axes labels
            hValidClusterSize[layerIndex][m][i]->GetXaxis()->SetTitle("Cluster size");
            hValidClusterSize[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
            // draw histograms
            TString expr(Form("cluster_size>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 
            tree->Draw(expr,cond);
  
            hValidClusterSize[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Cluster size distributions"));
            hValidClusterSize[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Check", "Should have peak in range 2-4"));
            hValidClusterSize[layerIndex][m][i]->Write(histoName);
        } // side loop ends
     } // type loop ends
    cValidSizeDist->Write(canvasName);
  } // layers loop ends

//----------------------------------------------------------------------
// Clusters: Cluster charge plots     
//
 int minCharge = 0;
 int maxCharge = 300;
  
 for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValidChargeDistribution%d", layer));
    TString canvasTitle(Form("Cluster Charge Distributions, Layer %d", layer));
    TString pdfFilename(Form("ClusterChargesvd%d.pdf", layer));
    TCanvas *cValidClusterCharge;
    cValidClusterCharge = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidClusterCharge->Divide(2,2);
    cValidClusterCharge->Draw();
    int k=0;	    
    for (int  m=1; m>=0; m--) { // loop over types
        for (int i=0; i<=1; i++) { //loop over sydes
	        TString histoTitle(Form("Validation: Cluster charge, layer %d, %s, %s side", layer,Type[m],Side[i]));
            TString histoName(Form("hValidCharge%s_Layer%d_%s",Side[i], layer,Type[m]));
	        k=k+1;
            cValidClusterCharge->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
	        // build histogram
            hValidClusterCharge[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minCharge,maxCharge);
            // set axes labels
            hValidClusterCharge[layerIndex][m][i]->GetXaxis()->SetTitle("Cluster charge (# of electrons)");
            hValidClusterCharge[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
            // draw histograms
            TString expr(Form("cluster_charge>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 
            gStyle->SetOptStat(101010);
            tree->Draw(expr,cond);
 
	        hValidClusterCharge[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Cluster charge distributions."));
            hValidClusterCharge[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Check", "Should have peak at around 5000 electrons"));
            hValidClusterCharge[layerIndex][m][i]->Write(histoName);
        } // types loop ends
    } // side loop ends
            cValidClusterCharge->Write(canvasName);
    //cValidClusterCharge->Print(pdfFilename);
 } // layer loop ends
	    
//---------------------------------------------------------------------
// Clusters: Seed charge plots     
//
 int minSeedCharge = 0;
 int maxSeedCharge = 300;
  
 for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValidSeedDistribution%d", layer));
    TString canvasTitle(Form("Seed Charge Distributions, Layer %d", layer));
    TString pdfFilename(Form("SeedChargesvd%d.pdf", layer));
    TCanvas *cValidSeedCharge;
    cValidSeedCharge = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidSeedCharge->Divide(2,2);
    cValidSeedCharge->Draw();
    int k=0;	    
    for (int  m=1; m>=0; m--) { // loop over types
        for (int i=0; i<=1; i++) { //loop over sydes
	        TString histoTitle(Form("Validation: Seed charge, layer %d, %s, %s side", layer,Type[m],Side[i]));
            TString histoName(Form("hValidSeedCharge%s_Layer%d_%s",Side[i], layer,Type[m]));
	        k=k+1;
            cValidSeedCharge->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
	        // build histogram
            hValidSeedCharge[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minSeedCharge,maxSeedCharge);
            // set axes labels
            hValidSeedCharge[layerIndex][m][i]->GetXaxis()->SetTitle("Seed charge (# of electrons)");
            hValidSeedCharge[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
            // draw histograms
            TString expr(Form("cluster_seedCharge>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 
           
            tree->Draw(expr,cond);
 
	        hValidSeedCharge[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Seed charge distributions"));
            hValidSeedCharge[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Check", "Should have peak at around 6000 electrons"));
            hValidSeedCharge[layerIndex][m][i]->Write(histoName);
        } // sides loop ends
    } // types loop ends
    cValidSeedCharge->Write(canvasName);
    // cValidSeedCharge->Print(pdfFilename);
} // layer loop ends

//--------------------------------------------------------------------
// Simhits: dE/dx plots 
    
 int mindEdx = 0;
 float maxdEdx = 10;

 for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValiddEdxDistribution%d", layer));
    TString canvasTitle(Form("SimHit dE/dx Distributions, Layer %d", layer));
    TCanvas *cValidEnergyLoss;
    cValidEnergyLoss = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidEnergyLoss->Divide(2,1);
    cValidEnergyLoss->Draw();
    int k=0;	    
    for (int  m=1; m>=0; m--) { // loop over types
	        TString histoTitle(Form("Validation: Simhit dE/dx, layer %d, %s", layer,Type[m]));
            TString histoName(Form("hValiddEdx_Layer%d_%s", layer,Type[m]));
            k=k+1;
 	        cValidEnergyLoss->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
            // build histogram
            hValidSimhitdEdX[layerIndex][m] = new TH1F(histoName, histoTitle, bins,mindEdx,maxdEdx);
            // set axes labels
            hValidSimhitdEdX[layerIndex][m]->GetXaxis()->SetTitle("dE/dx for simhits [MeV/cm]");
            hValidSimhitdEdX[layerIndex][m]->GetYaxis()->SetTitle("counts");
            // draw histograms
            TString expr(Form("simhit_dEdx*1000>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&sensor_type==%d", layer, m)); 
            
            treeSimhit->Draw(expr,cond);
            
	        hValidSimhitdEdX[layerIndex][m]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: dE/dx distributions"));
            hValidSimhitdEdX[layerIndex][m]->GetListOfFunctions()->Add(new TNamed("Check", "Should be landau distributed with MPV around 2.8"));
            hValidSimhitdEdX[layerIndex][m]->Write(histoName);
    } // types loop ends
    cValidEnergyLoss->Write(canvasName);
} // layer loop ends

//--------------------------------------------------------------------
// Truehits: truehit energy lost vs Truehit energy deposition plots 

 int minEnLost = 0;
 float maxEnLost = 0.001;
 //int minEnDep = 0;
 //float maxEnDep = 0.001;

 for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValidTrueHitDepChargevsEnLost%d", layer));
    TString canvasTitle(Form("Truehits: energy lost vs energy deposition, Layer %d", layer));
    TCanvas *cValidDepChargevsEnLost;
    cValidDepChargevsEnLost = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidDepChargevsEnLost->Divide(2,1);
    cValidDepChargevsEnLost->Draw();
    int k=0;
    for (int  m=1; m>=0; m--) { // loop over types

            TString histoTitle(Form("Validation: Truehits: energy lost vs energy deposition, layer %d, %s", layer,Type[m]));
            TString histoName(Form("hValidTrueHitDepChargevsEnLost_Layer%d_%s",layer,Type[m]));
            k=k+1;
            cValidDepChargevsEnLost->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
            // build histogram
            hValidTrueHitDepChargevsEnLost[layerIndex][m] = new TH2F(histoName, histoTitle, bins,minEnLost,maxEnLost, bins, minEnLost, maxEnLost);
            // set axes labels
            hValidTrueHitDepChargevsEnLost[layerIndex][m]->GetXaxis()->SetTitle("Lost momentum (eV)");
            hValidTrueHitDepChargevsEnLost[layerIndex][m]->GetYaxis()->SetTitle("True deposit energy (eV)");
            // draw histograms
            TString expr(Form("truehit_deposEnergy:truehit_lossmomentum>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&sensor_type==%d", layer, m)); 

            tree->Draw(expr,cond,"colz");

            TLine *line = new TLine(minEnLost,minEnLost,maxEnLost,maxEnLost);
	        //line->SetLineWidth(2);
	        line->SetLineColor(kRed);
	        line->Draw();

            hValidTrueHitDepChargevsEnLost[layerIndex][m]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: For the truehits: simulated energy lost in the detector vs reconstructed energy deposition"));
            hValidTrueHitDepChargevsEnLost[layerIndex][m]->GetListOfFunctions()->Add(new TNamed("Check", "Should be below the red angle with slope = pi/4"));
            hValidTrueHitDepChargevsEnLost[layerIndex][m]->Write(histoName);
        } // side loop ends
    cValidDepChargevsEnLost->Write(canvasName);
} // layer loop ends



//-------------------------------------------------------------------
// Truehits: Efficiency for finding clusters

 int binsEff = 50;

 for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name
    TString canvasName(Form("cValidEfficiencyLayer%d", layer));
    TString canvasTitle(Form("EfficiencyClustersLayer %d", layer));
    TString pdfFilename(Form("SVDEfficiencyLayer%d.pdf", layer));
    TCanvas *cValidEfficiency;
    cValidEfficiency = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidEfficiency->Divide(2,2);
    cValidEfficiency->Draw();
    int k = 0; // counter to change subdivisions of canvas
    for (int  m=1; m>=0; m--) { // loop over types
        for (int i=0; i<=1; i++) { // loop over sides

            k = k+1;
            cValidEfficiency->cd(k);
            // build histograms
 	        if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 

            TString histoTitleTotal(Form("Validation: Truehits(i.e. total) vs theta vs phi: %s side, layer %d, %s",Side[i], layer,Type[m]));
            TString histoNameTotal(Form("hValidEfficiencyTotal_%s_Layer%d_%s",Side[i], layer, Type[m]));
            TString histoTitlePassed(Form("Validation: Cluster(i.e. passed) vs theta vs phi: %s side, layer %d, %s",Side[i], layer, Type[m]));
            TString histoNamePassed(Form("hValidEfficiencyPassed_%s_Layer%d_%s",Side[i], layer, Type[m]));

            TString histoTitleEff(Form("Validation: Efficiency vs theta vs phi: %s side, layer %d, %s",Side[i], layer, Type[m]));
            TString histoNameEff(Form("hValidEfficiency_%s_Layer%d_%s",Side[i], layer, Type[m]));
            
            hValidEffTotal[layerIndex][m][i] = new TH2F(histoNameTotal, histoTitleTotal, binsEff,minPhi,maxPhi,binsEff,minTheta,maxTheta);
            hValidEffPassed[layerIndex][m][i] = new TH2F(histoNamePassed, histoTitlePassed, binsEff,minPhi,maxPhi,binsEff,minTheta,maxTheta);

            // set axes labels
            //TString axisLabel(Form("%s",Angle[j]);
            hValidEffTotal[layerIndex][m][i]->GetXaxis()->SetTitle("#phi (phi)");
            hValidEffTotal[layerIndex][m][i]->GetYaxis()->SetTitle("#theta (theta)");
            hValidEffPassed[layerIndex][m][i]->GetXaxis()->SetTitle("#phi (phi)");
            hValidEffPassed[layerIndex][m][i]->GetYaxis()->SetTitle("#theta (theta)");
 
            // create strings for drawing 
	        TString exprTotal(Form("truehit_eff_theta:truehit_eff_phi>>%s",histoNameTotal.Data()));
	        TString exprPassed(Form("truehit_eff_theta:truehit_eff_phi>>%s",histoNamePassed.Data()));
            TString cutPassed( "truehit_eff_hascluster > 0.5"); // only the truehits with hascluster = 1 can make it 
            cutPassed.Append(Form("&&layer==%d&&sensor_type==%d", layer, m)); 
            TString cond(Form("layer==%d&&sensor_type==%d", layer, m)); 

            if (i==0) {
	        treeUEff->Draw(exprPassed,cutPassed);
            treeUEff->Draw(exprTotal,cond);
            }
            else {
	        treeVEff->Draw(exprPassed,cutPassed);
            treeVEff->Draw(exprTotal,cond);
            }

            // create TEfficiency object 
            TEfficiency* pEff = 0;
            
            // check consistency of histograms 
            if(TEfficiency::CheckConsistency(*hValidEffPassed[layerIndex][m][i],*hValidEffTotal[layerIndex][m][i]))
            {
            pEff = new TEfficiency(*hValidEffPassed[layerIndex][m][i],*hValidEffTotal[layerIndex][m][i]);
            pEff->Draw("colz");
	        pEff->SetTitle(histoTitleEff);
            pEff->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Signal to noise ratio for the digits"));
            pEff->GetListOfFunctions()->Add(new TNamed("Check", "Should have peak around 20"));
            pEff->Write(histoNameEff);
               }
            gStyle->SetOptStat(10);
            //gStyle->SetNumberContours(30);
            //gStyle->SetOptStat("e");
            //gStyle->SetStatY(0.95);
            //gStyle->SetStatX(44)

        } //sides loop ends
    } //types loop ends   
    cValidEfficiency->Write(canvasName);
    //cValidEfficiency->Print(pdfFilename);
 } //layers loop ends



//-------------------------------------------------------------------
// Digits: Signal to Noise plots 
    
  int minSNRatio = 0;
  int maxSNRatio = 1;
 
  for (int layer=layerMin; layer<=layerMax; layer++) { // loop on layers  
    int layerIndex = layerToIndex(layer);
    // build canvas name 
    TString canvasName(Form("cValidDigitSignalToNoiseLayer%d", layer));
    TString canvasTitle(Form("Digits: Signal to Noise Ratio, Layer %d", layer));
    TString pdfFilename(Form("SNsvd%d.pdf", layer));
    TCanvas *cValidSN;
    cValidSN = new TCanvas(canvasName,canvasTitle,canvasWidth,canvasHeight);
    cValidSN->Divide(2,2);
    cValidSN->Draw();
    int k=0;        
    for (int  m=1; m>=0; m--) { // loop over types
        for (int i=0; i<=1; i++) { // loop over sides
            TString histoTitle(Form("Validation: Digits: Signal to Noise Ratio, Layer %d, %s side", layer,Type[m],Side[i]));
            TString histoName(Form("hValidDigitSignalToNoise%s_Layer%d_%s",Side[i], layer,Type[m]));
            k=k+1;            
            cValidSN->cd(k);
            if ((layer == 3) && (m == 0)){continue;} // skip slanted histos for layer 3 
            // build histogram
            hValidDigitSignalToNoise[layerIndex][m][i] = new TH1F(histoName, histoTitle, bins,minSNRatio,maxSNRatio);
            // set axes labels
            hValidDigitSignalToNoise[layerIndex][m][i]->GetXaxis()->SetTitle("Signal to noise ratio");
            hValidDigitSignalToNoise[layerIndex][m][i]->GetYaxis()->SetTitle("counts");
            // draw histograms
            TString expr(Form("digit_charge/digit_noise>>%s",histoName.Data()));
            TString cond(Form("layer==%d&&strip_dir==%d&&sensor_type==%d", layer, i, m)); 

	        treeDigit->Draw(expr,cond);
  
            hValidDigitSignalToNoise[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Description", "Validation: Signal to noise ratio for the digits"));
            hValidDigitSignalToNoise[layerIndex][m][i]->GetListOfFunctions()->Add(new TNamed("Check", "Should have peak around 20"));
            hValidDigitSignalToNoise[layerIndex][m][i]->Write(histoName);
        } // side loop ends
     } // type loop ends
    cValidSN->Write(canvasName);
    // cValidSN->Print(pdfFilename);
  } // layers loop ends


}//function end

  

