#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>

#include "TH3.h"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include "TFile.h"
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <string>
// #include <TRandom3.h>
// #include <TMinuit.h>
#include "TH1.h"
#include "TF1.h"
#include "TH2.h"
#include "TF2.h"
// #include "TGraphErrors.h"
// #include "TStyle.h"
#include "TMath.h"
// #include "TMatrixDSym.h"
#include "TFitResult.h"
// #include "TLegend.h"
// #include "TColor.h"
// #include "TPaveText.h"
// #include "TPaveStats.h"
// #include "TGraphAsymmErrors.h"
// #include "TMacro.h"
// #include "THStack.h"
// #include "TLegendEntry.h"
// #include "TDatime.h"
#include "TString.h"
// #include "TStyle.h"
#include "TLatex.h"
// #include "TRandom3.h"
// #include "TGraphPainter.h"
#include <algorithm>
#include <functional>




using namespace Belle2;

std::vector<double> NoKickCuts::cutSelector(double sinTheta, double momentum, int layer1, int layer2, parameter par)
{
  std::vector<double> out;
  for (int i = 0; i < 2; i++) {
    double cut = getCut(layer1, layer2, par, (minmax)i, Norm) /
                 (sqrt(sinTheta) * pow(momentum, getCut(layer1, layer2, par, (minmax)i, Pow))
                  + getCut(layer1, layer2, par, (minmax)i, Bkg));
    out.push_back(cut);
  }
  return out;
}

double NoKickCuts::getCut(int layer1, int layer2, parameter par, minmax m, cutname cut)
{
  std::vector<std::vector <std::vector <std::vector<double>>>> cutVector;
  double out;

  switch (cut) {
    case 0:
      cutVector = m_cutNorm;
      break;

    case 1:
      cutVector = m_cutPow;
      break;

    case 2:
      cutVector = m_cutBkg;
      break;
  }
  //numeration order inside vector<vector<vector<vetor<double>>>> cut:
  //<minmax<parameters<layer_int<layer_ext>>>>
  //minmax: 0=min,1=max
  //parameters: 0=omega, 1=d0, 2=phi0, 3=z0, 4=tanLmabda


  out = cutVector.at(m).at(par).at(layer1).at(layer2);
  return out;
}

void NoKickCuts::FillCuts(std::string m_fileName)
{
  TFile* file = TFile::Open(m_fileName.c_str());


  TH3F* input_norm_m = (TH3F*)file->Get("output_norm_m");
  TH3F* input_pow_m = (TH3F*)file->Get("output_pow_m");
  TH3F* input_bkg_m = (TH3F*)file->Get("output_bkg_m") ;

  TH3F* input_norm_M = (TH3F*)file->Get("output_norm_M");
  TH3F* input_pow_M = (TH3F*)file->Get("output_pow_M");
  TH3F* input_bkg_M = (TH3F*)file->Get("output_bkg_M");


  for (int mm = 0; mm < 2; mm++) {
    std::vector<std::vector<std::vector<double>>> m_cutNorm_minmax;
    std::vector<std::vector<std::vector<double>>> m_cutPow_minmax;
    std::vector<std::vector<std::vector<double>>> m_cutBkg_minmax;
    for (int par = 0; par < m_nbinpar; par++) {
      std::vector<std::vector<double>> m_cutNorm_par;
      std::vector<std::vector<double>> m_cutPow_par;
      std::vector<std::vector<double>> m_cutBkg_par;
      for (int lay1 = 0; lay1 < m_nbinlay; lay1++) {
        std::vector<double> m_cutNorm_lay1;
        std::vector<double> m_cutPow_lay1;
        std::vector<double> m_cutBkg_lay1;
        for (int lay2 = 0; lay2 < m_nbinlay; lay2++) {
          int binNumber = input_norm_m->GetBin(par, lay1, lay2);

          if (mm == 0) {
            m_cutNorm_lay1.push_back(input_norm_m->GetBinContent(binNumber));
            m_cutPow_lay1.push_back(input_pow_m->GetBinContent(binNumber));
            m_cutBkg_lay1.push_back(input_bkg_m->GetBinContent(binNumber));
          }
          if (mm == 1) {
            m_cutNorm_lay1.push_back(input_norm_M->GetBinContent(binNumber));
            m_cutPow_lay1.push_back(input_pow_M->GetBinContent(binNumber));
            m_cutBkg_lay1.push_back(input_bkg_M->GetBinContent(binNumber));
          }
        }
        m_cutNorm_par.push_back(m_cutNorm_lay1);
        m_cutPow_par.push_back(m_cutPow_lay1);
        m_cutBkg_par.push_back(m_cutBkg_lay1);
        m_cutNorm_lay1.clear();
        m_cutPow_lay1.clear();
        m_cutBkg_lay1.clear();
      }
      m_cutNorm_minmax.push_back(m_cutNorm_par);
      m_cutPow_minmax.push_back(m_cutPow_par);
      m_cutBkg_minmax.push_back(m_cutBkg_par);
      m_cutNorm_par.clear();
      m_cutPow_par.clear();
      m_cutBkg_par.clear();
    }
    m_cutNorm.push_back(m_cutNorm_minmax);
    m_cutPow.push_back(m_cutPow_minmax);
    m_cutBkg.push_back(m_cutBkg_minmax);
    m_cutNorm_minmax.clear();
    m_cutPow_minmax.clear();
    m_cutBkg_minmax.clear();
  }
  file->Close();


}
