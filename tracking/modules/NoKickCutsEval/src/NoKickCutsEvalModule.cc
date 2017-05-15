#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <TFile.h>
#include <tracking/modules/hitToTrueXP/hitToTrueXPModule.h>
#include <tracking/modules/NoKickCutsEval/NoKickCutsEvalModule.h>


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
#include "TH3.h"
#include "TF1.h"
#include "TH2.h"
#include "TF2.h"
// #include "TGraphErrors.h"
// #include "TStyle.h"
#include "TMath.h"
// #include "TMatrixDSym.h"
#include "TFitResult.h"
//#include "TLegend.h"
// #include "TColor.h"
// #include "TPaveText.h"
// #include "TPaveStats.h"
// #include "TGraphAsymmErrors.h"
// #include "TMacro.h"
// #include "THStack.h"
//#include "TLegendEntry.h"
// #include "TDatime.h"
#include "TString.h"
// #include "TStyle.h"
#include "TLatex.h"
// #include "TRandom3.h"
// #include "TGraphPainter.h"
#include <algorithm>
#include <functional>


using namespace Belle2;

REG_MODULE(NoKickCutsEval)

NoKickCutsEvalModule::NoKickCutsEvalModule() : Module()
{
  setDescription("This module evaluate cuts necessary for the selction of reco tracks based on Multiple Scattering, NoKickRTSel");
}


NoKickCutsEvalModule::~NoKickCutsEvalModule()
{
}



void NoKickCutsEvalModule::initialize()
{

  // lim_temp.push_back(0.0003 * ext_lim);
  // lim_temp.push_back(0.3 * ext_lim);
  // lim_temp.push_back(0.05 * ext_lim);
  // lim_temp.push_back(0.2 * ext_lim);
  // lim_temp.push_back(0.5 * 0.3 * ext_lim);
  lim_temp.push_back(0.4 * ext_lim);
  lim_temp.push_back(1. * ext_lim);
  lim_temp.push_back(0.3 * ext_lim);
  lim_temp.push_back(1. * ext_lim);
  lim_temp.push_back(0.3 * ext_lim);


  for (int par = 0; par < nbinpar; par++) {
    std::vector<std::vector<std::vector<std::vector<TH1F*>>>> histo_par;
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      std::vector<std::vector<std::vector<TH1F*>>> histo_lay1;
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
        std::vector<std::vector<TH1F*>> histo_lay2;
        for (int theta = 0; theta < nbint; theta++) {
          std::vector<TH1F*> histo_theta;
          for (int p = 0; p < nbinp; p++) {
            histo_theta.push_back(new TH1F("histo_" + name_par.at(par) + Form("_layer%d-%d_theta%d_p%d", lay1, lay2, theta, p),
                                           "histo_" + name_par.at(par) + Form("_layer%d-%d_theta%d_p%d", lay1, lay2, theta, p), nbin, -lim_temp.at(par), lim_temp.at(par)));
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
    histo.push_back(histo_par);
    histo_par.clear();
  }

//iniziialize of useful store array and relations
  StoreArray<SVDCluster> storeClusters("");
  StoreArray<SVDTrueHit> storeTrueHits("");
  StoreArray<MCParticle> storeMCParticles("");
  StoreArray<RecoTrack> recoTracks("");
  // StoreArray<TrackCandidates> TrackCandidates("");

  storeClusters.isRequired();
  storeTrueHits.isRequired();
  storeMCParticles.isRequired();
  recoTracks.isRequired();
  // TrackCandidates.isRequired();


  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray recoTracksToMCParticles(recoTracks , storeMCParticles);
  // RelationArray TrackCandidatesToMCParticles(TrackCandidates , storeMCParticles );



//inizialize output TFile with cuts-histograms
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
    std::vector<hitToTrueXP> XP8 = m_trackSel.m_8hitTrack;
    bool PriorCut = m_trackSel.globalCut(XP8);
    // std::cout << "size XP8=" << XP8.size() << std::endl;
    // int countee =0;
    // for( auto ele : XP8){
    //   countee++;
    //   std::cout <<" count=" << countee;
    //   std::cout << ", layer=" << ele.getSensorLayer() << std::endl;
    // }
    // std::cout << "----------------------------------" << std::endl;

    m_trackSel.m_8hitTrack.clear();
    m_trackSel.m_hitToTrueXP.clear();
    m_trackSel.m_setHitToTrueXP.clear();
    if (!PriorCut) {GlobCounter++; continue;}

    if (XP8.size() > 0) {
      for (int i = 0; (i + 1) < (int)XP8.size(); i++) {
        for (int par = 0; par < nbinpar; par++) {
          int p = (int)((XP8.at(i).m_momentum0.Mag() - pmin) / pwidth);
          if (p > nbinp - 1 || p < 0) {
            pCounter++;
            continue;
          }
          double sinTheta = abs(XP8.at(i).m_momentum0.Y()) / sqrt(pow(XP8.at(i).m_momentum0.Y(), 2) + pow(XP8.at(i).m_momentum0.Z(), 2));
          int t = (int)((asin(sinTheta) - tmin) / pwidth);
          if (t > nbint - 1 || t < 0) {
            tCounter++;
            continue;
          }
          double deltaPar = deltaParEval(XP8.at(i), XP8.at(i + 1), (parameters)par);
          if (deltaPar == OVER) continue;
          histo.at(par).at(XP8.at(i).m_sensorLayer).at(XP8.at(i + 1).m_sensorLayer).at(t).at(p)->Fill(deltaPar);
          if (i == 0) {
            deltaPar = deltaParEval(XP8.at(i), XP8.at(i), (parameters)par, true);
            if (deltaPar == OVER)continue;
            histo.at(par).at(0).at(XP8.at(i).m_sensorLayer).at(t).at(p)->Fill(deltaPar);
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

  for (int par = 0; par < nbinpar; par++) {
    std::vector<std::vector<std::vector<std::vector<double>>>> cut_M_par;
    std::vector<std::vector<std::vector<std::vector<double>>>> cut_m_par;
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      std::vector<std::vector<std::vector<double>>> cut_M_lay1;
      std::vector<std::vector<std::vector<double>>> cut_m_lay1;
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
        std::vector<std::vector<double>> cut_M_lay2;
        std::vector<std::vector<double>> cut_m_lay2;
        for (int theta = 0; theta < nbint; theta++) {
          std::vector<double> cut_M_theta;
          std::vector<double> cut_m_theta;
          for (int p = 0; p < nbinp; p++) {

            //--------------first method to evaluate cuts -------------------------//
            //  TF1 *fit_2gaus  = new TF1("fit_2gaus","[0]*TMath::Gaus(x,[1],[2], kTRUE)+[3]*TMath::Gaus(x,[4],[5],kTRUE)+[6]",-limit.at(par).at(lay1).at(lay2).at(theta).at(p),limit.at(par).at(lay1).at(lay2).at(theta).at(p));
            //
            //  int N= histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Integral();
            //  double integral= histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Integral();
            //  int bin0 = nbin/2;
            //  int n =histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin0);
            //  double molt_sigma1=0.5;
            //  double molt_sigma2=2;
            //  double sigma1=(double)molt_sigma1*(histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetStdDev());
            //  double sigma2 =(double)molt_sigma2*(histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetStdDev());
            //  double norm1 = sigma1*sqrt(2*3.1415)*0.9*n;
            //  double norm2 = sigma2*sqrt(2*3.1415)*0.1*n;
            //  double mean1=(double)histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetMean();
            //  double mean2=(double)histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetMean();
            //  double bkg = (double)histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(2);
            //
            //  fit_2gaus->SetParameters(norm1,mean1,sigma1,norm2,mean2,sigma2,bkg);
            //  histo.at(par).at(lay1).at(lay2).at(theta).at(p) -> Fit(fit_2gaus,"","",-limit.at(par).at(lay1).at(lay2).at(theta).at(p),limit.at(par).at(lay1).at(lay2).at(theta).at(p));
            //  //double limit_M =3*(sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))+fit_2gaus->GetParameter(1));
            //   cut_M_theta.push_back(3*(sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))));
            //  // limit_m =3*(-sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))+fit_2gaus->GetParameter(1));
            //  cut_m_theta.push_back(3*(-sqrt(fit_2gaus->GetParameter(2)*fit_2gaus->GetParameter(2))));
            //---------------END of first method ------------------------//


            //--------second method to evaluate (without fit)-----------------//
            double integral = histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Integral();
            double sum_M = histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(nbin + 1);
            double sum_m = histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(0);
            double percent = 1 - cutFunction(p, pwidth);

            int bin_m = 0;
            int bin_M = nbin + 1;
            while (sum_m < integral * percent / 2) {
              bin_m++;
              sum_m = sum_m + histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin_m);
            }
            while (sum_M < integral * percent / 2) {
              bin_M--;
              sum_M = sum_M + histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinContent(bin_M);
            }
            if (histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetEntries() < 100) {
              bin_m = 0;
              bin_M = nbin + 1;
            }
            cut_M_theta.push_back(histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinCenter(bin_M));
            cut_m_theta.push_back(histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetBinCenter(bin_m));
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

  //buld 2D histogram with cuts
  std::vector<std::vector<std::vector<TH2F*>>> cut_M_histo;
  std::vector<std::vector<std::vector<TH2F*>>> cut_m_histo;

  for (int par = 0; par < nbinpar; par++) {
    std::vector<std::vector<TH2F*>> cut_M_histo_par;
    std::vector<std::vector<TH2F*>> cut_m_histo_par;
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      std::vector<TH2F*> cut_M_histo_lay1;
      std::vector<TH2F*> cut_m_histo_lay1;
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
        cut_M_histo_lay1.push_back(new TH2F("CUTS_M_" + name_par.at(par) + Form("layer%d_%d", lay1, lay2),
                                            "CUTS_M_" + name_par.at(par) + Form("_layer%d_%d", lay1, lay2), nbinp, pmin, pmax, nbint, tmin, tmax));
        cut_m_histo_lay1.push_back(new TH2F("CUTS_m_" + name_par.at(par) + Form("layer%d_%d", lay1, lay2),
                                            "CUTS_m_" + name_par.at(par) + Form("_layer%d_%d", lay1, lay2), nbinp, pmin, pmax, nbint, tmin, tmax));
        for (int theta = 1; theta <= nbint; theta++) {
          for (int p = 1; p <= nbinp; p++) {
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

  //Fit
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_norm;
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_pow;
  std::vector<std::vector<std::vector<std::vector<double>>>> cut_out_bkg;

  for (int minmax = 0; minmax < 2; minmax++) {
    std::vector<std::vector<std::vector<double>>> cut_out_norm_minmax;
    std::vector<std::vector<std::vector<double>>> cut_out_pow_minmax;
    std::vector<std::vector<std::vector<double>>> cut_out_bkg_minmax;
    for (int par = 0; par < nbinpar; par++) {
      std::vector<std::vector<double>> cut_out_norm_par;
      std::vector<std::vector<double>> cut_out_pow_par;
      std::vector<std::vector<double>> cut_out_bkg_par;
      for (int lay1 = 0; lay1 < nbinlay; lay1++) {
        std::vector<double> cut_out_norm_lay1;
        std::vector<double> cut_out_pow_lay1;
        std::vector<double> cut_out_bkg_lay1;
        for (int lay2 = 0; lay2 < nbinlay; lay2++) {

          TF2* fit_MS  = new TF2("fit_MS", "[0]*1/(TMath::Power(x,[1])*TMath::Sqrt(TMath::Sin(y)))+[2]", pmin, pmax, tmin, tmax);

          double norm = cut_M.at(par).at(lay1).at(lay2).at(1).at(1);
          double Pow = 1;
          double bkg = cut_M.at(par).at(lay1).at(lay2).at(1).at(nbinp - 1);
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

  if (validationON) {

    TCanvas* c_empty = new TCanvas("c_empty", "c_empty", 800, 600);
    c_empty->cd();
    c_empty->Print("NoKickCuts_validation_plots.pdf[");

    std::vector<std::vector<std::vector<std::vector<TCanvas*>>>> c_fit;


    for (int minmax = 0; minmax < 2; minmax++) {
      std::vector<std::vector<std::vector<TCanvas*>>> c_fit_minmax;
      for (int par = 0; par < nbinpar; par++) {
        std::vector<std::vector<TCanvas*>> c_fit_par;
        for (int lay1 = 0; lay1 < nbinlay; lay1++) {
          std::vector<TCanvas*> c_fit_lay1;
          for (int lay2 = 0; lay2 < nbinlay; lay2++) {
            TString name("c_");
            name += minmax;
            name += "_";
            name += par;
            name += "_";
            name += lay1;
            name += "_";
            name += lay2;
            c_fit_lay1.push_back(new TCanvas(name, name, 800, 600));
          }
          c_fit_par.push_back(c_fit_lay1);
          c_fit_lay1.clear();
        }
        c_fit_minmax.push_back(c_fit_par);
        c_fit_par.clear();
      }
      c_fit.push_back(c_fit_minmax);
      c_fit_minmax.clear();
    }

    for (int minmax = 0; minmax < 2; minmax++) {
      for (int par = 0; par < nbinpar; par++) {
        for (int lay1 = 0; lay1 < nbinlay; lay1++) {
          for (int lay2 = 0; lay2 < nbinlay; lay2++) {

            c_fit.at(minmax).at(par).at(lay1).at(lay2)->cd();

            if (minmax == 0) {
              cut_m_histo.at(par).at(lay1).at(lay2)->Draw("LEGO2Z");
              cut_m_histo.at(par).at(lay1).at(lay2)->GetXaxis()->SetTitle("Momentum [GeV/c]");
              cut_m_histo.at(par).at(lay1).at(lay2)->GetYaxis()->SetTitle("#theta [rad]");
              cut_m_histo.at(par).at(lay1).at(lay2)->SetTitle(Form("Cuts Fit, max, par=%d, lay1=%d, lay2=%d", par, lay1, lay2));
              cut_m_histo.at(par).at(lay1).at(lay2)->GetYaxis()->SetRange(1, nbint - 1);
              cut_m_histo.at(par).at(lay1).at(lay2)->GetXaxis()->SetRange(1, nbinp - 1);
            }
            if (minmax == 1) {
              cut_M_histo.at(par).at(lay1).at(lay2)->Draw("LEGO2Z");
              cut_M_histo.at(par).at(lay1).at(lay2)->GetXaxis()->SetTitle("Momentum [GeV/c]");
              cut_M_histo.at(par).at(lay1).at(lay2)->GetYaxis()->SetTitle("#theta [rad]");
              cut_M_histo.at(par).at(lay1).at(lay2)->SetTitle(Form("Cuts Fit, min, par=%d, lay1=%d, lay2=%d", par, lay1, lay2));
              cut_M_histo.at(par).at(lay1).at(lay2)->GetYaxis()->SetRange(1, nbint - 1);
              cut_M_histo.at(par).at(lay1).at(lay2)->GetXaxis()->SetRange(1, nbinp - 1);
            }
            if (lay1 > 2 && lay2 > 2) {
              if (lay2 - lay1 > 0 && ((lay2 - lay1) < 3 || (lay1 == 0 && lay2 == 3)))
                c_fit.at(minmax).at(par).at(lay1).at(lay2)->Print("NoKickCuts_validation_plots.pdf");
            }
          }
        }
      }
    }

//--------------uncomment to obtain an example of distribution ------------------//
    // std::vector<std::vector<TCanvas*>> c_dist_145;
    // for(int theta =0; theta<=nbint; theta++){
    //   std::vector<TCanvas *> c_dist_theta;
    //   for(int p =0; p<=nbinp; p++){
    //     TString name("c_145");
    //     name += "_";
    //     name += theta;
    //     name += "_";
    //     name += p;
    //     c_dist_theta.push_back(new TCanvas(name,name,800,600));
    //   }
    //   c_dist_145.push_back(c_dist_theta);
    //   c_dist_theta.clear();
    // }
    //
    // for(int theta =0; theta<=nbint; theta++){
    //   for(int p =0; p<=nbinp; p++){
    //     c_dist_145.at(theta).at(p)->cd();
    //     histo.at(1).at(4).at(5).at(theta).at(p)->Draw();
    //     histo.at(1).at(4).at(5).at(theta).at(p)->GetXaxis()->SetTitle("Delta parameter [??]");
    //     histo.at(1).at(4).at(5).at(theta).at(p)->GetYaxis()->SetTitle("Number of Events");
    //     histo.at(1).at(4).at(5).at(theta).at(p)->SetTitle(Form("Delta Parameters distribution, par=%d, lay1=%d, lay2=%d, theta=%d, p=%d",1,4,5,theta,p));
    //     c_dist_145.at(theta).at(p)->Print("NoKickCuts_validation_plots.pdf");
    //   }
    // }









    double plot_dist = 0; //=1 causes crash
    if (plot_dist == 1) {
      std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<TCanvas*>>>>>> c_dist;
      for (int minmax = 0; minmax < 2; minmax++) {
        std::vector<std::vector<std::vector<std::vector<std::vector<TCanvas*>>>>> c_dist_minmax;
        for (int par = 0; par < nbinpar; par++) {
          std::vector<std::vector<std::vector<std::vector<TCanvas*>>>> c_dist_par;
          for (int lay1 = 0; lay1 < nbinlay; lay1++) {
            std::vector<std::vector<std::vector<TCanvas*>>> c_dist_lay1;
            for (int lay2 = 0; lay2 < nbinlay; lay2++) {
              std::vector<std::vector<TCanvas*>> c_dist_lay2;
              for (int theta = 0; theta < nbint; theta++) {
                std::vector<TCanvas*> c_dist_theta;
                for (int p = 0; p < nbinp; p++) {
                  TString name("c_");
                  name += minmax;
                  name += "_";
                  name += par;
                  name += "_";
                  name += lay1;
                  name += "_";
                  name += lay2;
                  name += "_";
                  name += theta;
                  name += "_";
                  name += p;
                  c_dist_theta.push_back(new TCanvas(name, name, 800, 600));
                }
                c_dist_lay2.push_back(c_dist_theta);
                c_dist_theta.clear();
              }
              c_dist_lay1.push_back(c_dist_lay2);
              c_dist_lay2.clear();
            }
            c_dist_par.push_back(c_dist_lay1);
            c_dist_lay1.clear();
          }
          c_dist_minmax.push_back(c_dist_par);
          c_dist_par.clear();
        }
        c_dist.push_back(c_dist_minmax);
        c_dist_minmax.clear();
      }

      for (int minmax = 0; minmax < 2; minmax++) {
        for (int par = 0; par < nbinpar; par++) {
          for (int lay1 = 0; lay1 < nbinlay; lay1++) {
            for (int lay2 = 0; lay2 < nbinlay; lay2++) {
              if (minmax == 0) {
                for (int theta = 0; theta < nbint; theta++) {
                  for (int p = 0; p < nbinp; p++) {
                    c_dist.at(minmax).at(par).at(lay1).at(lay2).at(theta).at(p)->cd();
                    histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Draw();
                    histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetXaxis()->SetTitle("Delta parameter [??]");
                    histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetYaxis()->SetTitle("Number of Events");
                    histo.at(par).at(lay1).at(lay2).at(theta).at(p)->SetTitle(
                      Form("Delta Parameters distribution, par=%d, lay1=%d, lay2=%d, theta=%d, p=%d", par, lay1, lay2, theta, p));
                    if (lay1 == 4 && lay2 == 5 && par == 1)
                      if (abs(lay1 - lay2) < 2) c_dist.at(minmax).at(par).at(lay1).at(lay2).at(theta).at(p)->Print("NoKickCuts_validation_plots.pdf");
                  }
                }
              }
            }
          }
        }
      }
    }


    c_empty->cd();
    c_empty->Print("NoKickCuts_validation_plots.pdf]");

  }

  //------------------_DEBUG------------//
  for (int g = 0; g < nbinp; g++) {
    double p_out_mom = g * pwidth + pmin;
    double t_out_theta = twidth + tmin;
    std::cout << "momentum=" << p_out_mom << std::endl;

    std::cout << "d0, 3-4,  Min: " << cut_m.at(1).at(3).at(4).at(1).at(g) << std::endl;
    std::cout << "min cut (TH2F):" << cut_m_histo.at(1).at(3).at(4)->GetBinContent(g + 1, 2) << std::endl;
    double norm_min = cut_out_norm.at(0).at(1).at(3).at(4);
    double pow_min = cut_out_pow.at(0).at(1).at(3).at(4);
    double bkg_min = cut_out_bkg.at(0).at(1).at(3).at(4);
    std::cout << "norm par min:" << norm_min << std::endl;
    std::cout << "pow par min:" << pow_min << std::endl;
    std::cout << "bkg par min:" << bkg_min << std::endl;
    std::cout << "evaluate min cut:" << norm_min / (sqrt(sin(t_out_theta)) * pow(p_out_mom, pow_min)) + bkg_min << std::endl;

    std::cout << "d0, 3-4,  Max: " << cut_M.at(1).at(3).at(4).at(1).at(g) << std::endl;
    std::cout << "max cut (TH2F):" << cut_M_histo.at(1).at(3).at(4)->GetBinContent(g + 1, 2) << std::endl;
    double norm_max = cut_out_norm.at(1).at(1).at(3).at(4);
    double pow_max = cut_out_pow.at(1).at(1).at(3).at(4);
    double bkg_max = cut_out_bkg.at(1).at(1).at(3).at(4);
    std::cout << "norm par max:" << norm_max << std::endl;
    std::cout << "pow par max:" << pow_max << std::endl;
    std::cout << "bkg par max:" << bkg_max << std::endl;
    std::cout << "evaluate max cut:" << norm_max / (sqrt(sin(t_out_theta)) * pow(p_out_mom, pow_max)) + bkg_max << std::endl;
    std::cout << "----------------------------------------" << std::endl;
  }

  m_outputFile->cd();
  for (int par = 0; par < nbinpar; par++) {
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
        for (int theta = 0; theta < nbint; theta++) {
          for (int p = 0; p < nbinp; p++) {
            double layerdiff = lay2 - lay1;
            if (layerdiff >= 0 && (layerdiff < 3 || (lay1 == 0 && lay2 == 3))) {
              if (histo.at(par).at(lay1).at(lay2).at(theta).at(p)->GetEntries() > 0) {
                histo.at(par).at(lay1).at(lay2).at(theta).at(p)->Write();
              }
            }
          }
        }
      }
    }
  }
  for (int par = 0; par < nbinpar; par++) {
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
        for (int minmax = 0; minmax < 2; minmax++) {
          if (minmax == 0) {
            double layerdiff = lay2 - lay1;
            if (layerdiff >= 0 && (layerdiff < 3 || (lay1 == 0 && lay2 == 3))) {
              cut_m_histo.at(par).at(lay1).at(lay2)->Write();
            }
          }
          if (minmax == 1) {
            double layerdiff = lay2 - lay1;
            if (layerdiff >= 0 && (layerdiff < 3 || (lay1 == 0 && lay2 == 3))) {
              cut_M_histo.at(par).at(lay1).at(lay2)->Write();
            }
          }
        }
      }
    }
  }
  //END OF DEBUG -------------_//


  //--------------------------------OUTPUT: histogram booking, filling -------------------//

  TH3F* output_norm_m = new TH3F("output_norm_m", "output_norm_m", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);
  TH3F* output_pow_m = new TH3F("output_pow_m", "output_pow_m", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);
  TH3F* output_bkg_m = new TH3F("output_bkg_m", "output_bkg_m", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);

  TH3F* output_norm_M = new TH3F("output_norm_M", "output_norm_M", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);
  TH3F* output_pow_M = new TH3F("output_pow_M", "output_pow_M", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);
  TH3F* output_bkg_M = new TH3F("output_bkg_M", "output_bkg_M", nbinpar, 0, 4, nbinlay, 0, 4, nbinlay, 0, 4);


  for (int par = 0; par < nbinpar; par++) {
    for (int lay1 = 0; lay1 < nbinlay; lay1++) {
      for (int lay2 = 0; lay2 < nbinlay; lay2++) {
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
  //m_outputFile->Close();
  std::cout << "number of spacepoint with theta out of limits=" << tCounter << std::endl;
  std::cout << "number of spacepoint with momentum out of limits=" << pCounter << std::endl;
  std::cout << "number of tracks cutted by global cuts=" << GlobCounter << std::endl;


}

void NoKickCutsEvalModule::terminate() {}
