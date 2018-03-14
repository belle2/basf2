/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <TFile.h>
#include <tracking/modules/trackingPerformanceEvaluation/HitXPModule.h>
#include <tracking/modules/vxdtfRedesign/NoKickCutsEvalModule.h>


#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <string>
#include "TH1.h"
#include "TH3.h"
#include "TF1.h"
#include "TH2.h"
#include "TF2.h"
#include "TMath.h"
#include "TFitResult.h"
#include "TString.h"
#include "TLatex.h"
#include <algorithm>
#include <functional>


using namespace Belle2;

REG_MODULE(NoKickCutsEval)

NoKickCutsEvalModule::NoKickCutsEvalModule() : Module()
{
  setDescription("This module evaluate cuts necessary for the selction of reco tracks based on Multiple Scattering, NoKickRTSel");

  /** write validation plots */
  addParam("useValidation", c_validationON,
           "print in output file validation plot: track parameters distributions and cuts distributions", false);

  /** use fit method to evaluate cuts: DO NOT USE! */
  addParam("useFitMethod", c_fitMethod, "apply the method of double-gaussian fit to evaluate the cuts", false);
}


NoKickCutsEvalModule::~NoKickCutsEvalModule()
{
}



void NoKickCutsEvalModule::initialize()
{

  m_histoLim.push_back(0.4 * c_multLimit);
  m_histoLim.push_back(1. * c_multLimit);
  m_histoLim.push_back(0.3 * c_multLimit);
  m_histoLim.push_back(1. * c_multLimit);
  m_histoLim.push_back(0.3 * c_multLimit);


  for (int par = 0; par < c_nbinpar; par++) {
    std::vector<std::vector<std::vector<std::vector<TH1F*>>>> histo_par;
    for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
      std::vector<std::vector<std::vector<TH1F*>>> histo_lay1;
      for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
        std::vector<std::vector<TH1F*>> histo_lay2;
        for (int theta = 0; theta < c_nbint; theta++) {
          std::vector<TH1F*> histo_theta;
          for (int p = 0; p < c_nbinp; p++) {
            histo_theta.push_back(new TH1F("histo_" + m_namePar.at(par) + Form("_layer%d-%d_theta%d_p%d", lay1, lay2, theta, p),
                                           "histo_" + m_namePar.at(par) + Form("_layer%d-%d_theta%d_p%d", lay1, lay2, theta, p), c_nbin, -m_histoLim.at(par),
                                           m_histoLim.at(par)));
          }
          histo_lay2.push_back(histo_theta);
          histo_theta.clear();
        }
        histo_lay1.push_back(histo_lay2);
        histo_lay2.clear();
      }
      histo_par.push_back(histo_lay1);
      histo_lay1.clear();
    }
    m_histo.push_back(histo_par);
    histo_par.clear();
  }

  /** iniziialize of useful store array and relations */
  StoreArray<SVDCluster> storeClusters("");
  StoreArray<SVDTrueHit> storeTrueHits("");
  StoreArray<MCParticle> storeMCParticles("");
  StoreArray<RecoTrack> recoTracks("");

  storeClusters.isRequired();
  storeTrueHits.isRequired();
  storeMCParticles.isRequired();
  recoTracks.isRequired();

  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray recoTracksToMCParticles(recoTracks , storeMCParticles);



  /** inizialize output TFile with cuts-histograms */
  m_outputFile = new TFile("NoKickCuts.root", "RECREATE");




}

void NoKickCutsEvalModule::beginRun() {}

void NoKickCutsEvalModule::event()
{
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;
  StoreArray<MCParticle> MCParticles;
  StoreArray<RecoTrack> recoTracks;

  for (const RecoTrack& track : recoTracks) {
    m_trackSel.hit8TrackBuilder(track);
    std::vector<hitXP> XP8 = m_trackSel.m_8hitTrack;
    bool PriorCut = m_trackSel.globalCut(XP8);
    m_trackSel.m_8hitTrack.clear();
    m_trackSel.m_hitXP.clear();
    m_trackSel.m_setHitXP.clear();
    if (!PriorCut) {m_globCounter++; continue;}

    if (XP8.size() > 0) {
      for (int i = 0; (i + 1) < (int)XP8.size(); i++) {
        for (int par = 0; par < c_nbinpar; par++) {
          int p = (int)((XP8.at(i).m_momentum0.Mag() - c_pmin) / c_pwidth);
          if (p > c_nbinp - 1 || p < 0) {
            m_pCounter++;
            continue;
          }
          double sinTheta = abs(XP8.at(i).m_momentum0.Y()) / sqrt(pow(XP8.at(i).m_momentum0.Y(), 2) + pow(XP8.at(i).m_momentum0.Z(), 2));
          int t = (int)((asin(sinTheta) - c_tmin) / c_pwidth);
          if (t > c_nbint - 1 || t < 0) {
            m_tCounter++;
            continue;
          }
          double deltaPar = deltaParEval(XP8.at(i), XP8.at(i + 1), (NoKickCuts::EParameters)par);
          if (deltaPar == c_over) continue;
          m_histo.at(par).at(XP8.at(i).m_sensorLayer).at(XP8.at(i + 1).m_sensorLayer).at(t).at(p)->Fill(deltaPar);
          if (i == 0) {
            deltaPar = deltaParEval(XP8.at(i), XP8.at(i), (NoKickCuts::EParameters)par, true);
            if (deltaPar == c_over)continue;
            m_histo.at(par).at(0).at(XP8.at(i).m_sensorLayer).at(t).at(p)->Fill(deltaPar);
          }
        }
      }
    }
  }
}




void NoKickCutsEvalModule::endRun()
{
  //-------------------------------FIT-EVALUATE THE CUTS---------------------------------------------------//

  std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> cut_m;
  std::vector<std::vector<std::vector<std::vector<std::vector<double>>>>> cut_M;

  for (int par = 0; par < c_nbinpar; par++) {
    std::vector<std::vector<std::vector<std::vector<double>>>> cut_M_par;
    std::vector<std::vector<std::vector<std::vector<double>>>> cut_m_par;
    for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
      std::vector<std::vector<std::vector<double>>> cut_M_lay1;
      std::vector<std::vector<std::vector<double>>> cut_m_lay1;
      for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
        std::vector<std::vector<double>> cut_M_lay2;
        std::vector<std::vector<double>> cut_m_lay2;
        for (int theta = 0; theta < c_nbint; theta++) {
          std::vector<double> cut_M_theta;
          std::vector<double> cut_m_theta;
          for (int p = 0; p < c_nbinp; p++) {

            //--------------first method to evaluate cuts, not used -------------------------//
            if (c_fitMethod) {
              TF1* fit_2gaus  = new TF1("fit_2gaus", "[0]*TMath::Gaus(x,[1],[2], kTRUE)+[3]*TMath::Gaus(x,[4],[5],kTRUE)+[6]",
                                        -m_histoLim.at(par), m_histoLim.at(par));

              int bin0 = c_nbin / 2;
              int nbin0 = m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin0);
              double moltSigma1 = 0.5;
              double moltSigma2 = 2;
              double sigma1 = (double)moltSigma1 * (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetStdDev());
              double sigma2 = (double)moltSigma2 * (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetStdDev());
              double norm1 = sigma1 * sqrt(2 * 3.1415) * 0.9 * nbin0;
              double norm2 = sigma2 * sqrt(2 * 3.1415) * 0.1 * nbin0;
              double mean1 = (double)m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetMean();
              double mean2 = (double)m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetMean();
              double bkg = (double)m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(2);

              fit_2gaus->SetParameters(norm1, mean1, sigma1, norm2, mean2, sigma2, bkg);
              m_histo.at(par).at(lay1).at(lay2).at(theta).at(p) -> Fit(fit_2gaus, "", "", -m_histoLim.at(par), m_histoLim.at(par));
              cut_M_theta.push_back(3 * (sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))));
              cut_m_theta.push_back(3 * (-sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))));
            }
            //---------------END of first method ------------------------//

            else {
              //--------second method to evaluate (without fit), used-----------------//
              double integral = m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Integral();
              double sum_M = m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(c_nbin + 1);
              double sum_m = m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(0);
              double percent = 1 - cutFunction(p, c_pwidth);

              int bin_m = 0;
              int bin_M = c_nbin + 1;
              while (sum_m < integral * percent / 2) {
                bin_m++;
                sum_m = sum_m + m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin_m);
              }
              while (sum_M < integral * percent / 2) {
                bin_M--;
                sum_M = sum_M + m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin_M);
              }
              if (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetEntries() < 100) {
                int filledBin_m = 0;
                int filledBin_M = c_nbin + 1;
                while (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(filledBin_m) == 0 && filledBin_m < (double)  c_nbin / 2) {
                  filledBin_m++;
                }
                while (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(filledBin_M) == 0 && filledBin_M > (double) c_nbin / 2) {
                  filledBin_M--;
                }
                bin_m = filledBin_m;
                bin_M = filledBin_M;
              }
              cut_M_theta.push_back(m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinCenter(bin_M));
              cut_m_theta.push_back(m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinCenter(bin_m));
            }
            //-----------------------END of second method ------------------------//

          }
          cut_M_lay2.push_back(cut_M_theta);
          cut_M_theta.clear();
          cut_m_lay2.push_back(cut_m_theta);
          cut_m_theta.clear();
        }
        cut_M_lay1.push_back(cut_M_lay2);
        cut_M_lay2.clear();
        cut_m_lay1.push_back(cut_m_lay2);
        cut_m_lay2.clear();
      }
      cut_M_par.push_back(cut_M_lay1);
      cut_M_lay1.clear();
      cut_m_par.push_back(cut_m_lay1);
      cut_m_lay1.clear();
    }
    cut_M.push_back(cut_M_par);
    cut_M_par.clear();
    cut_m.push_back(cut_m_par);
    cut_m_par.clear();
  }

  //------------------------------------------FIT THE CUTS --------------------------------//

  /** buld 2D histogram with cuts */
  std::vector<std::vector<std::vector<TH2F*>>> cut_M_histo;
  std::vector<std::vector<std::vector<TH2F*>>> cut_m_histo;

  for (int par = 0; par < c_nbinpar; par++) {
    std::vector<std::vector<TH2F*>> cut_M_histo_par;
    std::vector<std::vector<TH2F*>> cut_m_histo_par;
    for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
      std::vector<TH2F*> cut_M_histo_lay1;
      std::vector<TH2F*> cut_m_histo_lay1;
      for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
        cut_M_histo_lay1.push_back(new TH2F("CUTS_M_" + m_namePar.at(par) + Form("layer%d_%d", lay1, lay2),
                                            "CUTS_M_" + m_namePar.at(par) + Form("_layer%d_%d", lay1, lay2), c_nbinp, c_pmin, c_pmax, c_nbint, c_tmin, c_tmax));
        cut_m_histo_lay1.push_back(new TH2F("CUTS_m_" + m_namePar.at(par) + Form("layer%d_%d", lay1, lay2),
                                            "CUTS_m_" + m_namePar.at(par) + Form("_layer%d_%d", lay1, lay2), c_nbinp, c_pmin, c_pmax, c_nbint, c_tmin, c_tmax));
        for (int theta = 1; theta <= c_nbint; theta++) {
          for (int p = 1; p <= c_nbinp; p++) {
            cut_M_histo_lay1.at(lay2)->SetBinContent(p, theta, cut_M.at(par).at(lay1).at(lay2).at(theta - 1).at(p - 1));
            cut_m_histo_lay1.at(lay2)->SetBinContent(p, theta, cut_m.at(par).at(lay1).at(lay2).at(theta - 1).at(p - 1));
          }
        }
      }
      cut_M_histo_par.push_back(cut_M_histo_lay1);
      cut_M_histo_lay1.clear();
      cut_m_histo_par.push_back(cut_m_histo_lay1);
      cut_m_histo_lay1.clear();
    }
    cut_M_histo.push_back(cut_M_histo_par);
    cut_M_histo_par.clear();
    cut_m_histo.push_back(cut_m_histo_par);
    cut_m_histo_par.clear();
  }

  /** Fit */
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_norm;
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_pow;
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_bkg;

  for (int minmax = 0; minmax < 2; minmax++) {
    std::vector<std::vector<std::vector<double>>> cut_out_norm_minmax;
    std::vector<std::vector<std::vector<double>>> cut_out_pow_minmax;
    std::vector<std::vector<std::vector<double>>> cut_out_bkg_minmax;
    for (int par = 0; par < c_nbinpar; par++) {
      std::vector<std::vector<double>> cut_out_norm_par;
      std::vector<std::vector<double>> cut_out_pow_par;
      std::vector<std::vector<double>> cut_out_bkg_par;
      for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
        std::vector<double> cut_out_norm_lay1;
        std::vector<double> cut_out_pow_lay1;
        std::vector<double> cut_out_bkg_lay1;
        for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {

          TF2* fit_MS  = new TF2("fit_MS", "[0]*1/(TMath::Power(x,[1])*TMath::Sqrt(TMath::Sin(y)))+[2]", c_pmin, c_pmax, c_tmin, c_tmax);

          double norm = cut_M.at(par).at(lay1).at(lay2).at(1).at(1);
          double Pow = 1;
          double bkg = cut_M.at(par).at(lay1).at(lay2).at(1).at(c_nbinp - 1);
          fit_MS->SetParameters(norm, Pow, bkg);

          if (minmax == 0) {
            cut_m_histo.at(par).at(lay1).at(lay2) -> Fit("fit_MS");

          }
          if (minmax == 1) {
            cut_M_histo.at(par).at(lay1).at(lay2) -> Fit("fit_MS");

          }
          cut_out_norm_lay1.push_back(fit_MS->GetParameter(0));
          cut_out_pow_lay1.push_back(fit_MS->GetParameter(1));
          cut_out_bkg_lay1.push_back(fit_MS->GetParameter(2));
        }
        cut_out_norm_par.push_back(cut_out_norm_lay1);
        cut_out_norm_lay1.clear();
        cut_out_pow_par.push_back(cut_out_pow_lay1);
        cut_out_pow_lay1.clear();
        cut_out_bkg_par.push_back(cut_out_bkg_lay1);
        cut_out_bkg_lay1.clear();
      }
      cut_out_norm_minmax.push_back(cut_out_norm_par);
      cut_out_norm_par.clear();
      cut_out_pow_minmax.push_back(cut_out_pow_par);
      cut_out_pow_par.clear();
      cut_out_bkg_minmax.push_back(cut_out_bkg_par);
      cut_out_bkg_par.clear();
    }
    cut_out_norm.push_back(cut_out_norm_minmax);
    cut_out_norm_minmax.clear();
    cut_out_pow.push_back(cut_out_pow_minmax);
    cut_out_pow_minmax.clear();
    cut_out_bkg.push_back(cut_out_bkg_minmax);
    cut_out_bkg_minmax.clear();
  }

//----------------------------------------------VALIDATION PLOTS ------------------------------//

//-------------Some debugs lines-----------------//
  if (Belle2::LogSystem::Instance().isLevelEnabled(Belle2::LogConfig::c_Debug, 30, PACKAGENAME())) {
    for (int g = 0; g < c_nbinp; g++) {
      double p_out_mom = g * c_pwidth + c_pmin;
      double t_out_theta = c_twidth + c_tmin;
      B2DEBUG(30, "momentum=" << p_out_mom);
      B2DEBUG(30, "d0, 3-4,  Min: " << cut_m.at(1).at(3).at(4).at(1).at(g));
      B2DEBUG(30, "min cut (TH2F):" << cut_m_histo.at(1).at(3).at(4)->GetBinContent(g + 1, 2));
      double norm_min = cut_out_norm.at(0).at(1).at(3).at(4);
      double pow_min = cut_out_pow.at(0).at(1).at(3).at(4);
      double bkg_min = cut_out_bkg.at(0).at(1).at(3).at(4);
      B2DEBUG(30, "norm par min:" << norm_min);
      B2DEBUG(30, "pow par min:" << pow_min);
      B2DEBUG(30, "bkg par min:" << bkg_min);
      B2DEBUG(30, "evaluate min cut:" << norm_min / (sqrt(sin(t_out_theta)) * pow(p_out_mom, pow_min)) + bkg_min);
      B2DEBUG(30, "d0, 3-4,  Max: " << cut_M.at(1).at(3).at(4).at(1).at(g));
      B2DEBUG(30, "max cut (TH2F):" << cut_M_histo.at(1).at(3).at(4)->GetBinContent(g + 1, 2));
      double norm_max = cut_out_norm.at(1).at(1).at(3).at(4);
      double pow_max = cut_out_pow.at(1).at(1).at(3).at(4);
      double bkg_max = cut_out_bkg.at(1).at(1).at(3).at(4);
      B2DEBUG(30, "norm par max:" << norm_max);
      B2DEBUG(30, "pow par max:" << pow_max);
      B2DEBUG(30, "bkg par max:" << bkg_max);
      B2DEBUG(30, "evaluate max cut:" << norm_max / (sqrt(sin(t_out_theta)) * pow(p_out_mom, pow_max)) + bkg_max);
      B2DEBUG(30, "----------------------------------------");
    }
  }
  //-----------end of debug lines ------//

  if (c_validationON == 1) {
    /** print on tfile distributions of DeltaX */
    m_outputFile->cd();
    for (int par = 0; par < c_nbinpar; par++) {
      for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
        for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
          for (int theta = 0; theta < c_nbint; theta++) {
            for (int p = 0; p < c_nbinp; p++) {
              double layerdiff = lay2 - lay1;
              if (layerdiff >= 0 && layerdiff < 3) {
                if (m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetEntries() > 0) {
                  m_histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Write();
                }
              }
            }
          }
        }
      }
    }

    /** print on tfile distributions of cuts and fits */
    for (int par = 0; par < c_nbinpar; par++) {
      for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
        for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
          for (int minmax = 0; minmax < 2; minmax++) {
            if (minmax == 0) {
              int layerdiff = lay2 - lay1;
              if (layerdiff >= 0 && layerdiff < 3) {
                cut_m_histo.at(par).at(lay1).at(lay2)->Write();
              }
            }
            if (minmax == 1) {
              int layerdiff = lay2 - lay1;
              if (layerdiff >= 0 && layerdiff < 3) {
                cut_M_histo.at(par).at(lay1).at(lay2)->Write();
              }
            }
          }
        }
      }
    }
  }

  //--------------------------------OUTPUT: histogram booking, filling -------------------//

  TH3F* output_norm_m = new TH3F("output_norm_m", "output_norm_m", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);
  TH3F* output_pow_m = new TH3F("output_pow_m", "output_pow_m", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);
  TH3F* output_bkg_m = new TH3F("output_bkg_m", "output_bkg_m", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);

  TH3F* output_norm_M = new TH3F("output_norm_M", "output_norm_M", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);
  TH3F* output_pow_M = new TH3F("output_pow_M", "output_pow_M", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);
  TH3F* output_bkg_M = new TH3F("output_bkg_M", "output_bkg_M", c_nbinpar, 0, 4, c_nbinlay, 0, 4, c_nbinlay, 0, 4);


  for (int par = 0; par < c_nbinpar; par++) {
    for (int lay1 = 0; lay1 < c_nbinlay; lay1++) {
      for (int lay2 = 0; lay2 < c_nbinlay; lay2++) {
        output_norm_m->SetBinContent(par, lay1, lay2, cut_out_norm.at(0).at(par).at(lay1).at(lay2));
        output_norm_M->SetBinContent(par, lay1, lay2, cut_out_norm.at(1).at(par).at(lay1).at(lay2));

        output_pow_m->SetBinContent(par, lay1, lay2, cut_out_pow.at(0).at(par).at(lay1).at(lay2));
        output_pow_M->SetBinContent(par, lay1, lay2, cut_out_pow.at(1).at(par).at(lay1).at(lay2));

        output_bkg_m->SetBinContent(par, lay1, lay2, cut_out_bkg.at(0).at(par).at(lay1).at(lay2));
        output_bkg_M->SetBinContent(par, lay1, lay2, cut_out_bkg.at(1).at(par).at(lay1).at(lay2));
      }
    }
  }

  m_outputFile->cd();
  output_norm_m->Write();
  output_norm_M->Write();
  output_pow_m->Write();
  output_pow_M->Write();
  output_bkg_m->Write();
  output_bkg_M->Write();
  m_outputFile->Close();
  delete m_outputFile;

  B2INFO("number of spacepoint with theta out of limits=" << m_tCounter);
  B2INFO("number of spacepoint with momentum out of limits=" << m_pCounter);
  B2INFO("number of tracks cutted by global cuts=" << m_globCounter);


}

void NoKickCutsEvalModule::terminate() {}


double NoKickCutsEvalModule::deltaParEval(hitXP hit1, hitXP hit2, NoKickCuts::EParameters par, bool is0)
{
  double out = c_over;
  int layer1 = hit1.m_sensorLayer;
  int layer2 = hit2.m_sensorLayer;
  double layerdiff = layer2 - layer1;
  if (layerdiff >= 0 && layerdiff < 3) {
    switch (par) {
      case NoKickCuts::c_Omega:
        out = abs(hit1.getOmegaEntry() - hit2.getOmegaEntry());
        if (is0) out = abs(hit1.getOmega0() - hit2.getOmegaEntry());
        break;

      case NoKickCuts::c_D0:
        out = hit1.getD0Entry() - hit2.getD0Entry();
        if (is0) out = hit1.getD00() - hit2.getD0Entry();
        break;

      case NoKickCuts::c_Phi0:
        out = asin(sin(hit1.getPhi0Entry())) - asin(sin(hit2.getPhi0Entry()));
        if (is0) out = asin(sin(hit1.getPhi00())) - asin(sin(hit2.getPhi0Entry()));
        break;

      case NoKickCuts::c_Z0:
        out = hit1.getZ0Entry() - hit2.getZ0Entry();
        if (is0) out = hit1.getZ00() - hit2.getZ0Entry();
        break;

      case NoKickCuts::c_Tanlambda:
        out = hit1.getTanLambdaEntry() - hit2.getTanLambdaEntry();
        if (is0) out = hit1.getTanLambda0() - hit2.getTanLambdaEntry();
        break;
    }
  }
  return out;
}

double NoKickCutsEvalModule::cutFunction(int p, double pwidth)
{
  double out;
  double mom = p * pwidth;
  if (mom > 0.04)
    out = -7.5 * pow(10, -7) / pow(mom, 3.88) + 1;
  else out = 6.3 * mom + 0.57;
  return out;
}
