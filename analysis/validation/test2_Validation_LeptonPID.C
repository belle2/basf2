/*
<header>
<input>../ana-dstars.root, ../ana-jpsiks.root</input>
<output></output>
<description>Bisection algorithm that calculates PID cuts for set efficiencies for electrons, muons and pions.</description>
<contact></contact>
</header>
*/
//Contributor: Marcel Hohmann (February,2017)
////////////////////////////////////////////////////////////////////////////////
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "TMath.h"


//Use 404 for error : eff outside eff domain. These functions remove this number from the final output.
void printNumbers(TPaveText *tpave, double number) {
  if (number > 403) {
    tpave ->AddText("---");
  } else if (number > 0.99) {
    tpave->AddText(TString::Format("1 - %.5g",1-number));
  }  else {
    tpave->AddText(TString::Format("%.5g",number));
  }
}

double histoFillNumbers(double number) {
  if (number > 403) {
    return 0;
  } else {
    return number;
  }
}

double cutEfficiency(TChain *chain, int amountPassedBaseCut, double cutval, TString cut, TString basecut) {
  TString cutstring = basecut + " && " + cut + TString::Format(" %.50f",cutval);
  TH1F *cuthisto = new TH1F("h1","h1",2,0,2);
  chain->Project("h1","exp_no",cutstring);

  int amountPassedCut = cuthisto->Integral();
  double efficiency = (double)amountPassedCut/(double)amountPassedBaseCut;
  delete cuthisto;

  return efficiency;
}

double PIDforTargetEff(double *finalEffValues, int maxIterations, double tolerance, double targetEff, TChain *targetChain, TChain *fakeChainOne, TChain *fakeChainTwo, TString targetCut, TString targetBaseCut, TString fakeCutOne, TString fakeBaseCutOne, TString fakeCutTwo, TString fakeBaseCutTwo) {
  // Get total amounts that pass the basecuts
  TH1F *baseTargetHisto  = new TH1F("baseTargetHisto", "baseTargetHisto" ,2,0,2);
  TH1F *baseFakeHistoOne = new TH1F("baseFakeHistoOne","baseFakeHistoOne",2,0,2);
  TH1F *baseFakeHistoTwo = new TH1F("baseFakeHistoTwo","baseFakeHistoTwo",2,0,2);

  targetChain  -> Project("baseTargetHisto" ,"exp_no", targetBaseCut);
  fakeChainOne -> Project("baseFakeHistoOne","exp_no",fakeBaseCutOne);
  fakeChainTwo -> Project("baseFakeHistoTwo","exp_no",fakeBaseCutTwo);

  int targetNumberPassedBaseCut  =  baseTargetHisto->Integral();
  int fakeOneNumberPassedBaseCut = baseFakeHistoOne->Integral();
  int fakeTwoNumberPassedBaseCut = baseFakeHistoTwo->Integral();

  delete baseTargetHisto;
  delete baseFakeHistoOne;
  delete baseFakeHistoTwo;

  //Find limits of efficiency for target
  double maxefficiency = cutEfficiency(targetChain, targetNumberPassedBaseCut, 0.0, targetCut, targetBaseCut);
  double minefficiency = cutEfficiency(targetChain, targetNumberPassedBaseCut, 1.0, targetCut +"=", targetBaseCut); //Need the equals, otherwise will always return 0.

  //std::cout << maxefficiency << " | " << minefficiency << std::endl;

  //Initialise Final Cut Value
  double finalcutval;

  if (targetEff < minefficiency || targetEff > maxefficiency) {
    //std::cout << "Target Eff not within Eff domain" <<std::endl;
    finalcutval = 404; //Error Code for TargetEff not within domain.
    finalEffValues[0] = 404;
    finalEffValues[1] = 404;
    finalEffValues[2] = 404;

  } else {
    // Calculate cut val for target efficiency
    double lowercutval = 0.0;
    double uppercutval = 1.0;
    int counter = 0;
    double currenteffval = 0;

    while (counter < maxIterations && TMath::Abs(currenteffval - targetEff) > tolerance) {
      double currentcutval = 0.5*(lowercutval+uppercutval);
      currenteffval = cutEfficiency(targetChain, targetNumberPassedBaseCut, currentcutval, targetCut, targetBaseCut);
      if  (TMath::Abs(currenteffval - targetEff) < tolerance) {
        lowercutval = currentcutval;
        uppercutval = currentcutval;
      } else if  (currenteffval < targetEff) {
        uppercutval = currentcutval;
      } else if (currenteffval > targetEff) {
        lowercutval = currentcutval;
      }
      counter++;
      //std::cout<< counter << " | " << currenteffval  << " | " <<  currentcutval <<   " | " <<  lowercutval << " | " << uppercutval << std::endl;
    }

    //Calculate final fake and real rates
    finalcutval = 0.5*(lowercutval+uppercutval);
    double finalTargetEff  = cutEfficiency(targetChain,  targetNumberPassedBaseCut,  finalcutval,  targetCut,  targetBaseCut);
    double finalFakeOneEff = cutEfficiency(fakeChainOne, fakeOneNumberPassedBaseCut, finalcutval, fakeCutOne, fakeBaseCutOne);
    double finalFakeTwoEff = cutEfficiency(fakeChainTwo, fakeTwoNumberPassedBaseCut, finalcutval, fakeCutTwo, fakeBaseCutTwo);

    //Pass Final Efficiency values out of function
    finalEffValues[0] = finalTargetEff;
    finalEffValues[1] = finalFakeOneEff;
    finalEffValues[2] = finalFakeTwoEff;
  }
  return finalcutval;
}



void test2_Validation_LeptonPID() {
  //Choose an output mode
  //Options: pdf: outputs results table in pdf || root: outputs results as histograms in root file. || csv: outputs results table to csv.
  //Can use multiple options if separated by space. Case Insensitive.
  std::string OutputMode = "root";

  //File Locations
  std::string mupath = "../";
  std::string  epath = "../";
  std::string pipath = "../";

// Using TChains allows for easy expansion to include more files
  std::vector<std::string> mufiles = {
    "ana-jpsiks.root"
  };
  std::vector<std::string>  efiles = {
    "ana-jpsiks.root"
  };
  std::vector<std::string> pifiles = {
    "ana-dstars.root"
  };

  //Tree Names
  std::string mutree = "jpsiks_mu";
  std::string  etree = "jpsiks_e";
  std::string pitree = "dsttree";

  //Add files to chains
  TChain muChain(mutree.c_str());
  TChain  eChain(etree.c_str());
  TChain piChain(pitree.c_str());

  for (unsigned int i = 0; i < mufiles.size(); i++) {
    mufiles[i] = mupath + mufiles[i];
    const char *cstr = mufiles[i].c_str();
    muChain.Add(cstr);
  }
  for (unsigned int i = 0; i < efiles.size(); i++) {
    efiles[i] = epath + efiles[i];
    const char *cstr = efiles[i].c_str();
    eChain.Add(cstr);
  }
  for (unsigned int i = 0; i < pifiles.size(); i++) {
    pifiles[i] = pipath + pifiles[i];
    const char *cstr = pifiles[i].c_str();
    piChain.Add(cstr);
  }

  //Cuts
  TString muBaseCut  = "abs(B0_Jpsi_M - 3.09692) < 0.05 && B0_Jpsi_mu1_PIDmu > 0.75"; // "abs(B0_Jpsi_M-3.09692)<0.093&&abs(B0_K_S0_M-0.49761)<0.024"
  TString eBaseCut   = "abs(B0_Jpsi_M - 3.09692) < 0.05 && B0_Jpsi_e1_PIDe > 0.75";
  TString piBaseCut  = "abs(DST_D0_M - 1.86484) < 0.012 && abs(DST_M - DST_D0_M - 0.14543) < 0.00075";
  TString muCutmu    = "B0_Jpsi_mu0_PIDmu >";
  TString muCute     = "B0_Jpsi_mu0_PIDe >";
  TString muCutpi    = "B0_Jpsi_mu0_PIDpi >";
  TString eCutmu     = "B0_Jpsi_e0_PIDmu >";
  TString eCute      = "B0_Jpsi_e0_PIDe >";
  TString eCutpi     = "B0_Jpsi_e0_PIDpi >";
  TString piCutmu    = "DST_D0_pi_PIDmu >";
  TString piCute     = "DST_D0_pi_PIDe >";
  TString piCutpi    = "DST_D0_pi_PIDpi >";
  //TString muPcut     = "B0_Jpsi_mu0_P > 1 && B0_Jpsi_mu0_P < 3";
  //TString ePcut      = "B0_Jpsi_e0_P > 1 && B0_Jpsi_e0_P < 3";
  //TString piPcut     = "DST_D0_pi_P > 1 && DST_D0_pi_P < 3";

  //Target Efficiencies
  std::vector<double> targetEfficiencies = {0.80,0.85,0.90,0.95,0.99};

  //Collect information
  double tolerance  = 0.001;
  int maxIterations = 50;

  unsigned int sizeTargetEff = targetEfficiencies.size();

  double muEfficiencies[sizeTargetEff][3];
  double  eEfficiencies[sizeTargetEff][3];
  double piEfficiencies[sizeTargetEff][3];
  double muCutValues[sizeTargetEff];
  double  eCutValues[sizeTargetEff];
  double piCutValues[sizeTargetEff];

  double tempefficiencies[3];

  for (unsigned int i = 0; i < targetEfficiencies.size(); i++) {
    muCutValues[i] = PIDforTargetEff(tempefficiencies, maxIterations, tolerance, targetEfficiencies[i], &muChain, &eChain, &piChain, muCutmu, muBaseCut, eCutmu, eBaseCut, piCutmu, piBaseCut);
    muEfficiencies[i][0] = tempefficiencies[0];
    muEfficiencies[i][1] = tempefficiencies[1];
    muEfficiencies[i][2] = tempefficiencies[2];
  }
  for (unsigned int i = 0; i < targetEfficiencies.size(); i++) {
    eCutValues[i] = PIDforTargetEff(tempefficiencies, maxIterations, tolerance, targetEfficiencies[i], &eChain, &muChain, &piChain, eCute, eBaseCut, muCute, muBaseCut, piCute, piBaseCut);
    eEfficiencies[i][0] = tempefficiencies[0];
    eEfficiencies[i][1] = tempefficiencies[1];
    eEfficiencies[i][2] = tempefficiencies[2];
  }
  for (unsigned int i = 0; i < targetEfficiencies.size(); i++) {
    piCutValues[i] = PIDforTargetEff(tempefficiencies, maxIterations, tolerance, targetEfficiencies[i], &piChain, &eChain, &muChain, piCutpi, piBaseCut, eCutpi, eBaseCut, muCutpi, muBaseCut);
    piEfficiencies[i][0] = tempefficiencies[0];
    piEfficiencies[i][1] = tempefficiencies[1];
    piEfficiencies[i][2] = tempefficiencies[2];
  }


  //Output Results
  if (strcasestr(OutputMode.c_str(), "pdf") != NULL) {
      //PS file name
      char pdf_filename[300] = "test2_Validation_LeptonPID.pdf";
      char pdf_filename_open[300];
      char pdf_filename_close[300];
      strcpy(pdf_filename_open,pdf_filename);
      strcpy(pdf_filename_close,pdf_filename);
      strcat(pdf_filename_open,"(");
      strcat(pdf_filename_close,")");
      std::cout <<"Creating PDF file: " << pdf_filename <<std::endl;
      TCanvas *printcanvas = new TCanvas("printcanvas");

      //Text on first page
      TPaveText *titletext = new TPaveText(0.05,0.1,0.95,0.9);
      titletext->SetBorderSize(0);
      titletext->SetFillColor(0);
      titletext->AddText("PID VALUES");
      titletext->AddText("Files:");
      titletext->AddText("mu:");
      for (unsigned int i = 0; i < mufiles.size(); i++) {
        titletext->AddText(mufiles[i].c_str());
      }
      titletext->AddText("e:");
      for (unsigned int i = 0; i < efiles.size(); i++) {
        titletext->AddText(efiles[i].c_str());
      }
      titletext->AddText("pi:");
      for (unsigned int i = 0; i < pifiles.size(); i++) {
        titletext->AddText(pifiles[i].c_str());
      }
      titletext->AddText("");
      titletext->AddText("Base Cuts:");
      titletext->AddText("mu: " +muBaseCut);
      titletext->AddText("e:  " + eBaseCut);
      titletext->AddText("pi: " +piBaseCut);
      titletext->Draw();
      //Open PDF and output first page
      printcanvas->Print(pdf_filename_open);

      // Create second page
      TCanvas *resultsPage = new TCanvas("resultspage","resultspage",1500,1200);

      // add results to TPaveText files

      TPaveText *muTpave[7];
      TPaveText *eTpave[7];
      TPaveText *piTpave[7];
      muTpave[0] = new TPaveText(0.02,0.87,0.18,0.92);
      eTpave[0] =  new TPaveText(0.02,0.56,0.18,0.61);
      piTpave[0] = new TPaveText(0.02,0.25,0.18,0.3);
      for (int i = 0; i < 6; i++) {
        muTpave[i+1] = new TPaveText(0.02+0.16*i,0.7,0.18+0.16*i,0.85);
        eTpave[i+1] = new TPaveText(0.02+0.16*i,0.39,0.18+0.16*i,0.54);
        piTpave[i+1] = new TPaveText(0.02+0.16*i,0.08,0.18+0.16*i,0.23);
      }

      //Headings:
      muTpave[0]->AddText("Mu:");
      muTpave[1]->AddText("Target Eff");
      muTpave[2]->AddText("PIDmu > ");
      muTpave[3]->AddText("mu Achieved Eff");
      muTpave[4]->AddText("|Target-Achieved|");
      muTpave[5]->AddText("e Fake Rate");
      muTpave[6]->AddText("pi Fake Rate");

      eTpave[0]->AddText("E:");
      eTpave[1]->AddText("Target Eff");
      eTpave[2]->AddText("PIDe > ");
      eTpave[3]->AddText("e Achieved Eff");
      eTpave[4]->AddText("|Target-Achieved|");
      eTpave[5]->AddText("mu Fake Rate");
      eTpave[6]->AddText("pi Fake Rate");

      piTpave[0]->AddText("Pi:");
      piTpave[1]->AddText("Target Eff");
      piTpave[2]->AddText("PIDpi > ");
      piTpave[3]->AddText("pi Achieved Eff");
      piTpave[4]->AddText("|Target-Achieved|");
      piTpave[5]->AddText("e Fake Rate");
      piTpave[6]->AddText("mu Fake Rate");

      //Add results:
      for (unsigned int i = 0; i < sizeTargetEff; i++) {
      printNumbers(muTpave[1], targetEfficiencies[i]);
      printNumbers(muTpave[2], muCutValues[i]);
      printNumbers(muTpave[3], muEfficiencies[i][0]);
      printNumbers(muTpave[4], TMath::Abs(targetEfficiencies[i] - muEfficiencies[i][0]));
      printNumbers(muTpave[5], muEfficiencies[i][1]);
      printNumbers(muTpave[6], muEfficiencies[i][2]);

      printNumbers(eTpave[1], targetEfficiencies[i]);
      printNumbers(eTpave[2], eCutValues[i]);
      printNumbers(eTpave[3], eEfficiencies[i][0]);
      printNumbers(eTpave[4], TMath::Abs(targetEfficiencies[i] - eEfficiencies[i][0]));
      printNumbers(eTpave[5], eEfficiencies[i][1]);
      printNumbers(eTpave[6], eEfficiencies[i][2]);

      printNumbers(piTpave[1], targetEfficiencies[i]);
      printNumbers(piTpave[2], piCutValues[i]);
      printNumbers(piTpave[3], piEfficiencies[i][0]);
      printNumbers(piTpave[4], TMath::Abs(targetEfficiencies[i] - piEfficiencies[i][0]));
      printNumbers(piTpave[5], piEfficiencies[i][1]);
      printNumbers(piTpave[6], piEfficiencies[i][2]);
      }

      //Draw Results
      for (int i = 0; i < 7; i++) {
        muTpave[i]->Draw("ndc");
        eTpave[i] ->Draw("ndc");
        piTpave[i]->Draw("ndc");
        muTpave[i]->SetFillColor(0);
        eTpave[i] ->SetFillColor(0);
        piTpave[i]->SetFillColor(0);
        muTpave[i]->SetTextAlign(21);
        eTpave[i] ->SetTextAlign(21);
        piTpave[i]->SetTextAlign(21);
        if (i > 0) {
          muTpave[i]->SetTextSize(0.02);
          eTpave[i] ->SetTextSize(0.02);
          piTpave[i]->SetTextSize(0.02);
        }
      }

      //Save and close PDF
      resultsPage->Print(pdf_filename_close);

  }
  if (strcasestr(OutputMode.c_str(), "root") != NULL) {

    TFile *rootOutputFile = new TFile("test2_Validation_LeptonPID.root","recreate");
    std::cout<< "creating root file: test2_Validation_LeptonPID.root"<< std::endl;

    TH1F *muPID = new TH1F("mu_PID","mu_PID;Efficiency;PIDmu>",sizeTargetEff,0,sizeTargetEff);
    TH1F *muAchEff = new TH1F("mu_AchEff","mu_AchEff;Efficiency;Achieved Efficiency",sizeTargetEff,0,sizeTargetEff);
    TH1F *muFakeE = new TH1F("mu_FakeE","mu_FakeE;Efficiency;Fake Rate E",sizeTargetEff,0,sizeTargetEff);
    TH1F *muFakePi = new TH1F("mu_FakePi","mu_FakePi;Efficiency;Fake Rate Pi",sizeTargetEff,0,sizeTargetEff);

    TH1F *ePID = new TH1F("e_PID","e_PID;Efficiency;PIDe>",sizeTargetEff,0,sizeTargetEff);
    TH1F *eAchEff = new TH1F("e_AchEff","e_AchEff;Efficiency;Achieved Efficiency",sizeTargetEff,0,sizeTargetEff);
    TH1F *eFakeMu = new TH1F("e_FakeMu","e_FakeMu;Efficiency;Fake Rate Mu",sizeTargetEff,0,sizeTargetEff);
    TH1F *eFakePi = new TH1F("e_FakePi","e_FakePi;Efficiency;Fake Rate Pi",sizeTargetEff,0,sizeTargetEff);

    TH1F *piPID = new TH1F("pi_PID","pi_PID;Efficiency;PIDpi>",sizeTargetEff,0,sizeTargetEff);
    TH1F *piAchEff = new TH1F("pi_AchEff","pi_AchEff;Efficiency;Achieved Efficiency",sizeTargetEff,0,sizeTargetEff);
    TH1F *piFakeE = new TH1F("pi_FakeE",";Efficiency;Fake Rate E",sizeTargetEff,0,sizeTargetEff);
    TH1F *piFakeMu = new TH1F("pi_FakeMu","pi_FakeMu;Efficiency;Fake Rate Mu",sizeTargetEff,0,sizeTargetEff);


    for (unsigned int i = 0; i< sizeTargetEff; i++) {
      muPID->SetBinContent(i+1, histoFillNumbers(muCutValues[i]));
      ePID ->SetBinContent(i+1,  histoFillNumbers(eCutValues[i]));
      piPID->SetBinContent(i+1, histoFillNumbers(piCutValues[i]));
      muAchEff->SetBinContent(i+1, histoFillNumbers(muEfficiencies[i][0]));
      eAchEff ->SetBinContent(i+1,  histoFillNumbers(eEfficiencies[i][0]));
      piAchEff->SetBinContent(i+1, histoFillNumbers(piEfficiencies[i][0]));
      muFakeE ->SetBinContent(i+1, histoFillNumbers(muEfficiencies[i][1]));
      muFakePi->SetBinContent(i+1, histoFillNumbers(muEfficiencies[i][2]));
      eFakeMu ->SetBinContent(i+1, histoFillNumbers(eEfficiencies[i][1]));
      eFakePi ->SetBinContent(i+1, histoFillNumbers(eEfficiencies[i][2]));
      piFakeE ->SetBinContent(i+1, histoFillNumbers(piEfficiencies[i][1]));
      piFakeMu->SetBinContent(i+1, histoFillNumbers(piEfficiencies[i][2]));

      muPID->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      ePID ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      piPID->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      muAchEff->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      eAchEff ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      piAchEff->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      muFakeE ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      muFakePi->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      eFakeMu ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      eFakePi ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      piFakeE ->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
      piFakeMu->GetXaxis()->SetBinLabel(i+1, TString::Format("%.2f",targetEfficiencies[i]));
    }

    muPID->GetListOfFunctions()->Add(new TNamed("Description","PIDmu cut for target efficiencies"));
    ePID ->GetListOfFunctions()->Add(new TNamed("Description","PIDe cut for target efficiencies" ));
    piPID->GetListOfFunctions()->Add(new TNamed("Description","PIDpi cut for target efficiencies"));
    muAchEff->GetListOfFunctions()->Add(new TNamed("Description","Achieved efficiency with PIDmu cut"));
    eAchEff ->GetListOfFunctions()->Add(new TNamed("Description","Achieved efficiency with PIDe cut" ));
    piAchEff->GetListOfFunctions()->Add(new TNamed("Description","Achieved efficiency with PIDpi cut"));
    muFakeE ->GetListOfFunctions()->Add(new TNamed("Description","Fraction of electrons that pass PIDmu cut"));
    muFakePi->GetListOfFunctions()->Add(new TNamed("Description","Fraction of pions that pass PIDmu cut" ));
    eFakeMu ->GetListOfFunctions()->Add(new TNamed("Description","Fraction of muons that pass PIDe cut" ));
    eFakePi ->GetListOfFunctions()->Add(new TNamed("Description","Fraction of pions that pass PIDe cut" ));
    piFakeE ->GetListOfFunctions()->Add(new TNamed("Description","Fraction of electrons that pass PIDpi cut"));
    piFakeMu->GetListOfFunctions()->Add(new TNamed("Description","Fraction of muons that pass PIDpi cut"));

    muPID->GetListOfFunctions()->Add(new TNamed("Check",""));
    ePID ->GetListOfFunctions()->Add(new TNamed("Check",""));
    piPID->GetListOfFunctions()->Add(new TNamed("Check",""));
    muAchEff->GetListOfFunctions()->Add(new TNamed("Check",""));
    eAchEff ->GetListOfFunctions()->Add(new TNamed("Check",""));
    piAchEff->GetListOfFunctions()->Add(new TNamed("Check",""));
    muFakeE ->GetListOfFunctions()->Add(new TNamed("Check",""));
    muFakePi->GetListOfFunctions()->Add(new TNamed("Check",""));
    eFakeMu ->GetListOfFunctions()->Add(new TNamed("Check",""));
    eFakePi ->GetListOfFunctions()->Add(new TNamed("Check",""));
    piFakeE ->GetListOfFunctions()->Add(new TNamed("Check",""));
    piFakeMu->GetListOfFunctions()->Add(new TNamed("Check",""));


    rootOutputFile->Write();
    rootOutputFile->Close();

  }
  if (strcasestr(OutputMode.c_str(), "csv") != NULL) {
      ofstream csvfile;
      csvfile.open("test2_Validation_LeptonPID.csv");
      std::cout << "Creating CSV file: test2_Validation_LeptonPID.csv" <<std::endl;

      csvfile << "Mu:\n";
      csvfile << "Target Efficiency,PIDmu >,mu Achieved Efficiency,|Target-Achieved|,e Fake Rate,pi Fake Rate\n";
      for (unsigned int i = 0; i < sizeTargetEff; i++) {
      csvfile << Form("%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",targetEfficiencies[i],histoFillNumbers(muCutValues[i]),histoFillNumbers(muEfficiencies[i][0]),histoFillNumbers(TMath::Abs(targetEfficiencies[i] - muEfficiencies[i][0])),histoFillNumbers(muEfficiencies[i][1]),histoFillNumbers(muEfficiencies[i][2]));
      }
      csvfile << "\n\nE:\n";
      csvfile << "Target Efficiency,PIDe >,e Achieved Efficiency,|Target-Achieved|,mu Fake Rate,pi Fake Rate\n";
      for (unsigned int i = 0; i < sizeTargetEff; i++) {
      csvfile << Form("%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",targetEfficiencies[i],histoFillNumbers(eCutValues[i]),histoFillNumbers(eEfficiencies[i][0]),histoFillNumbers(TMath::Abs(targetEfficiencies[i] - eEfficiencies[i][0])),histoFillNumbers(eEfficiencies[i][1]),histoFillNumbers(eEfficiencies[i][2]));
      }
      csvfile << "\n\nPi:\n";
      csvfile << "Target Efficiency,PIDpi >,pi Achieved Efficiency,|Target-Achieved|,e Fake Rate,mu Fake Rate\n";
      for (unsigned int i = 0; i < sizeTargetEff; i++) {
      csvfile << Form("%.10e,%.10e,%.10e,%.10e,%.10e,%.10e\n",targetEfficiencies[i],histoFillNumbers(piCutValues[i]),histoFillNumbers(piEfficiencies[i][0]),histoFillNumbers(TMath::Abs(targetEfficiencies[i] - piEfficiencies[i][0])),histoFillNumbers(piEfficiencies[i][1]),histoFillNumbers(piEfficiencies[i][2]));
      }
      csvfile.close();
  }
}
