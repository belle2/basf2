/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDClusterTimeShifterAlgorithm.h>

#include <svd/dbobjects/SVDClusterTimeShifter.h>

#include <TF1.h>
#include <TProfile.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>
#include <TFitResult.h>

using namespace Belle2;

SVDClusterTimeShifterAlgorithm::SVDClusterTimeShifterAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDClusterTimeShifterCollector")
  , m_id(str)
{
  setDescription("SVDClusterTimeShifter calibration algorithm");
}

CalibrationAlgorithm::EResult SVDClusterTimeShifterAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  auto payload = new Belle2::SVDClusterTimeShifter("SVDClusterTimeShifter_" + m_id);

  // double gaus fit function
  TF1* doubleGaus = new TF1("doubleGaus", myDoubleGaus, -25., 25., 7);
  doubleGaus->SetParName(0, "N");
  doubleGaus->SetParName(1, "f");
  doubleGaus->SetParName(2, "#mu_{1}");
  doubleGaus->SetParName(3, "#sigma_{1}");
  doubleGaus->SetParName(4, "#mu_{2}");
  doubleGaus->SetParName(5, "#sigma_{2}");
  doubleGaus->SetParName(6, "C");
  doubleGaus->SetParLimits(1, 0.7, 0.99);
  doubleGaus->SetParLimits(2, -m_allowedDeviationMean, m_allowedDeviationMean);
  // doubleGaus->SetParLimits(3, 1., 10.);
  doubleGaus->SetParLimits(4, -m_allowedDeviationMean, m_allowedDeviationMean);
  // doubleGaus->SetParLimits(5, 3., 50.);


  auto __hBinToSensorMap__ = getObjectPtr<TH1F>("__hBinToSensorMap__");

  for (auto alg : m_timeAlgorithms) {

    FileStat_t info;
    int cal_rev = 1;
    while (gSystem->GetPathInfo(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev), info) == 0)
      cal_rev++;
    std::unique_ptr<TFile> f(new TFile(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev), "RECREATE"));

    auto __hClusterSizeVsTimeResidual__ = getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).Data());

    // fir parameters in <side><size>
    std::map< char, std::map< int, std::vector<double> > > fitParameters;

    // Dummy fit to get the parameters
    for (int ij = 0; ij < (__hBinToSensorMap__->GetNbinsX()); ij++) {

      auto binLabel = __hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1);
      char side;
      int layer_num, sensor_num;
      std::sscanf(binLabel, "L%dS%dS%c", &layer_num, &sensor_num, &side);

      B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

      __hClusterSizeVsTimeResidual__->GetZaxis()->SetRange(ij + 1, ij + 1);
      auto hClusterSizeVsTimeResidual = (TH2D*)__hClusterSizeVsTimeResidual__->Project3D("yxe");
      hClusterSizeVsTimeResidual->SetDirectory(0);

      for (int size = 1; size <= (hClusterSizeVsTimeResidual->GetNbinsY()); size++) {

        if (auto searchSide = fitParameters.find(side); searchSide != fitParameters.end())
          if (auto searchSize = searchSide->second.find(size); searchSize != searchSide->second.end())
            if (int(searchSize->second.size())) continue;

        auto hist = (TH1D*)hClusterSizeVsTimeResidual->ProjectionX("tmp", size, size);
        hist->SetDirectory(0);

        doubleGaus->SetParameter(0, hist->GetSumOfWeights());
        doubleGaus->SetParameter(1, 0.9);
        doubleGaus->SetParameter(2, 0.);
        doubleGaus->SetParameter(3, hist->GetStdDev() * 0.75);
        doubleGaus->SetParameter(4, -1.);
        doubleGaus->SetParameter(5, hist->GetStdDev() * 3.);
        doubleGaus->SetParameter(6, 1.);

        int status = hist->Fit("doubleGaus", "SQ");
        if (!status)
          for (int par = 0; par < 7; par++)
            fitParameters[side][size].push_back(doubleGaus->GetParameter(par));
        delete hist;
      }
      delete hClusterSizeVsTimeResidual;
    }

    // map : shift values
    std::map< TString, std::vector<double> > shiftValues;

    for (int ij = 0; ij < (__hBinToSensorMap__->GetNbinsX()); ij++) {

      auto binLabel = __hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1);
      char side;
      int layer_num, sensor_num;
      std::sscanf(binLabel, "L%dS%dS%c", &layer_num, &sensor_num, &side);

      B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

      __hClusterSizeVsTimeResidual__->GetZaxis()->SetRange(ij + 1, ij + 1);
      auto hClusterSizeVsTimeResidual = (TH2D*)__hClusterSizeVsTimeResidual__->Project3D("yxe");

      hClusterSizeVsTimeResidual->SetName(Form("clusterSizeVsTimeResidual__L%dS%dS%c", layer_num, sensor_num, side));
      char sidePN = (side == 'U' ? 'P' : 'N');
      hClusterSizeVsTimeResidual->SetTitle(Form("ClusterSize vs Time Residual in %d.%d %c/%c", layer_num, sensor_num, side, sidePN));
      hClusterSizeVsTimeResidual->SetDirectory(0);
      f->cd();
      hClusterSizeVsTimeResidual->Write();

      for (int size = 1; size <= (hClusterSizeVsTimeResidual->GetNbinsY()); size++) {

        auto hist = (TH1D*)hClusterSizeVsTimeResidual->ProjectionX("tmp", size, size);
        hist->SetName(Form("clusterTimeResidual__L%dS%dS%c_Sz%d", layer_num, sensor_num, side, size));
        hist->SetTitle(Form("Cluster Time Residual for Size %d in %d.%d %c/%c", size, layer_num, sensor_num, side, sidePN));
        hist->SetDirectory(0);

        B2INFO("Histogram: " << hist->GetName() <<
               " Entries (n. clusters): " << hist->GetEntries());
        if (hist->GetEntries() < m_minEntries) {
          B2INFO("Histogram: " << hist->GetName() <<
                 " Entries (n. clusters): " << hist->GetEntries() <<
                 " Entries required: " << m_minEntries);
          B2WARNING("Not enough data, adding one run to the collector");
          f->Close();
          gSystem->Unlink(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev));
          return c_NotEnoughData;
        }

        if (auto searchSide = fitParameters.find(side); searchSide != fitParameters.end())
          if (auto searchSize = searchSide->second.find(size); searchSize != searchSide->second.end())
            for (int par = 0; par < 7; par++)
              doubleGaus->SetParameter(par, fitParameters[side][size][par]);
        doubleGaus->SetParameter(0, hist->GetSumOfWeights());
        doubleGaus->SetParameter(6, 1.);

        int status = hist->Fit("doubleGaus", "SQ");
        if (status) {
          B2WARNING("Fit failed for " << hist->GetName());
          f->Close();
          gSystem->Unlink(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev));
          return c_Failure;
        }

        shiftValues[binLabel].push_back(doubleGaus->GetParameter(2));
        if (std::fabs(shiftValues[binLabel].back()) > m_allowedDeviationMean) {
          B2WARNING("Shift valus is more than allowed in " << hist->GetName() << " : " << shiftValues[binLabel].back());
          f->Close();
          gSystem->Unlink(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev));
          return c_Failure;
        }

        f->cd();
        hist->Write();
        delete hist;
      }

      delete hClusterSizeVsTimeResidual;
    }

    for (auto item : shiftValues)
      payload->setClusterTimeShift(alg, item.first, item.second);

    f->Close();
  } // loop over algorithms

  saveCalibration(payload, "SVDClusterTimeShifter");

  return c_OK;
}
