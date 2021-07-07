/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingVXD/sectorMapTools/NoKickCuts.h>

#include "TH3.h"

#include "TFile.h"
#include <string>

using namespace Belle2;

std::vector<double> NoKickCuts::cutSelector(double sinTheta, double momentum, int layer1, int layer2, EParameters par)
{
  std::vector<double> out;
  for (int i = c_Min; i <= c_Max; i++) {
    double cut = getCut(layer1, layer2, par, (EMinMax)i, c_Norm) /
                 (sqrt(sinTheta) * pow(momentum, getCut(layer1, layer2, par, (EMinMax)i, c_Pow)))
                 + getCut(layer1, layer2, par, (EMinMax)i, c_Bkg);
    out.push_back(cut);
  }
  return out;
}

double NoKickCuts::getCut(int layer1, int layer2, EParameters par, EMinMax m, ECutName cut)
{
  std::vector<std::vector <std::vector <std::vector<double>>>> cutVector;
  double out;

  switch (cut) {
    case c_Norm:
      cutVector = m_cutNorm;
      break;

    case c_Pow:
      cutVector = m_cutPow;
      break;

    case c_Bkg:
      cutVector = m_cutBkg;
      break;
  }
  /** numeration order inside vector<vector<vector<vetor<double>>>> cut:
  * <EMinMax<EParameters<layer_int<layer_ext>>>>
  * EMinMax: 0=min,1=max
  * EParameters: 0=omega, 1=d0, 2=phi0, 3=z0, 4=tanLmabda
  */


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
  delete file;


}
