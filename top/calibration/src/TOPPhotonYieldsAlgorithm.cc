/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPPhotonYieldsAlgorithm.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/dbobjects/TOPCalPhotonYields.h>
#include <top/dbobjects/TOPCalChannelRQE.h>
#include <TROOT.h>
#include <TFile.h>
#include <TProfile.h>
#include <string>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPPhotonYieldsAlgorithm::TOPPhotonYieldsAlgorithm():
      CalibrationAlgorithm("TOPPhotonYieldsCollector")
    {
      setDescription("Calibration algorithm for photon pixel yields aimed for PMT ageing studies, for RQE calibration "
                     "and for finding optically decoupled PMT's");
    }


    CalibrationAlgorithm::EResult TOPPhotonYieldsAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // IoV for getting geometry payloads - they differ only between Run1 and Run2

      const auto& expRun = getRunList();
      updateDBObjPtrs(1, expRun[0].second, expRun[0].first);

      // geometry parameters

      auto* topgp = TOPGeometryPar::Instance();
      if (not topgp->isValid()) topgp->Initialize();
      if (not topgp->isValid()) {
        B2ERROR("TOPPhotonYieldsAlgorithm: cannot initialize geometry parameters");
        return c_Failure;
      }

      // construct file name and open output root file for storing merged histograms

      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "photonYields-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

      // create some additional histograms for monitoring of RQE calibration

      auto* h_rqe = new TH1F("RQE", "RQE distribution; RQE; entries/bin", 100, 0, m_maxRQE);
      auto* h_err = new TH1F("RQEerr", "RQE uncertainty distribution; RQE uncertainty; entries/bin", 100, 0, m_maxErrorRQE);

      // get basic histograms

      auto numTracks = getObjectPtr<TH1F>("numTracks");
      if (not numTracks) {
        B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'numTracks' not found");
        file->Close();
        return c_Failure;
      }
      numTracks->Write();

      auto timeStamp = getObjectPtr<TProfile>("timeStamp");
      if (not timeStamp) {
        B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'timeStamp' not found");
        file->Close();
        return c_Failure;
      }
      timeStamp->Write();

      // construct DB objects for storing the results and set the time stamp

      auto* dbPhotonYields = new TOPCalPhotonYields();
      dbPhotonYields->setTimeStamp(timeStamp->GetBinContent(1), timeStamp->GetBinError(1));
      auto* dbRQE = new TOPCalChannelRQE();

      // for each slot determine photon pixel yields, equalized alpha ratio and RQE, and store them in DB objects

      int numModules = numTracks->GetNbinsX();
      for (int slot = 1; slot <= numModules; slot++) {
        string slotName = (slot < 10) ? "_0" + to_string(slot) : "_" + to_string(slot);

        auto signalHits = getObjectPtr<TH1F>("signalHits" + slotName);
        if (not signalHits) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'signalHits' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        signalHits->Write();

        auto bkgHits = getObjectPtr<TH1F>("bkgHits" + slotName);
        if (not bkgHits) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'bkgHits' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        bkgHits->Write();

        auto effectiveSignalHits = getObjectPtr<TH1F>("effectiveSignalHits" + slotName);
        if (not effectiveSignalHits) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'effectiveSignalHits' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        effectiveSignalHits->Write();

        auto activePixels = getObjectPtr<TH1F>("activePixels" + slotName);
        if (not activePixels) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'activePixels' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        activePixels->Write();

        auto alphaLow = getObjectPtr<TH1F>("alphaLow" + slotName);
        if (not alphaLow) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'alphaLow' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        alphaLow->Sumw2();
        alphaLow->Write();

        auto alphaHigh = getObjectPtr<TH1F>("alphaHigh" + slotName);
        if (not alphaHigh) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'alphaHigh' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        alphaHigh->Sumw2();
        alphaHigh->Write();

        auto pulseHeights = getObjectPtr<TH2F>("pulseHeights" + slotName);
        if (not pulseHeights) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'pulseHeights' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        pulseHeights->Write();

        auto muonZ = getObjectPtr<TH1F>("muonZ" + slotName);
        if (not muonZ) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'muonZ' for slot " << slot << " not found");
          file->Close();
          delete dbPhotonYields;
          delete dbRQE;
          return c_Failure;
        }
        muonZ->Write();

        auto* photonYields = (TH1F*) signalHits->Clone("tmp1");
        photonYields->Add(signalHits.get(), bkgHits.get(), 1, -1); // subtract background
        for (int bin = 1; bin <= activePixels->GetNbinsX(); bin++) activePixels->SetBinError(bin, 0);
        photonYields->Divide(activePixels.get()); // normalize

        auto* bkgYields = (TH1F*) bkgHits->Clone("tmp2");
        bkgYields->Divide(activePixels.get()); // normalize

        auto* alphaRatio = (TH1F*) alphaHigh->Clone("tmp2");
        alphaRatio->Divide(alphaHigh.get(), alphaLow.get());
        equalize(alphaRatio);

        dbPhotonYields->set(slot, photonYields, bkgYields, alphaRatio, activePixels.get(), pulseHeights.get(), muonZ.get());

        effectiveSignalHits->Sumw2();
        effectiveSignalHits->Divide(activePixels.get()); // normalize
        const auto& channelMapper = topgp->getChannelMapper();
        for (int row = 0; row < 8; row++) {
          double muonCorr = getMuonCorrection(muonZ.get(), row);
          for (int col = 0; col < 64; col++) {
            int pixelID = row * 64 + col + 1;
            double mcYield = getNominalYield(slot, row, col) * topgp->getRelativePDEonMC(slot, pixelID) * muonCorr;
            if (mcYield == 0) continue; // this should not happen
            double rqe = effectiveSignalHits->GetBinContent(pixelID) / mcYield;
            double err = effectiveSignalHits->GetBinError(pixelID) / mcYield;
            if (rqe == 0) continue; // pixel dead or masked-out
            h_rqe->Fill(rqe);
            h_err->Fill(err);
            if (rqe > m_maxRQE or err > m_maxErrorRQE) continue;  // bad calibration, do not save it in payload
            unsigned channel = channelMapper.getChannel(pixelID);
            dbRQE->setRQE(slot, channel, rqe);
          }
        }

        delete photonYields;
        delete bkgYields;
        delete alphaRatio;
      }

      h_rqe->Write();
      h_err->Write();
      file->Close();

      saveCalibration(dbPhotonYields);
      saveCalibration(dbRQE);

      return c_OK;
    }


    double TOPPhotonYieldsAlgorithm::getEqualizingValue(int bin) const
    {
      // these constants are determined from fits to MC
      const double a[] = {1.50301, 1.16907, 1.00912, 0.943571, 0.889658, 0.93418, 1.01846, 0.953715};
      const double b[] = {-1.02531, -0.111269, 0.0731595, -0.121119, -0.79083, -1.00261, -1.41012, -1.23368};
      const double c[] = {8.47995, 3.18639, 1.9513, 2.58279, 4.79672, 5.5659, 7.558, 6.71336};
      const double d[] = {1.49743, 1.17224, 1.02211, 0.928502, 0.829334, 0.849355, 0.921871, 0.850427};

      int row = (bin - 1) / 64;
      if (row < 0 or row > 7) return 1;

      int col = (bin - 1) % 64 + 1;
      if (col == 1 or col == 64) return d[row];

      double x = (col - 32.5) / 64.;
      return a[row] + b[row] * pow(x, 2) + c[row] * pow(x, 4);
    }


    void TOPPhotonYieldsAlgorithm::equalize(TH1F* h) const
    {
      for (int bin = 1; bin <= h->GetNbinsX(); bin++) {
        double s = getEqualizingValue(bin);
        h->SetBinContent(bin, h->GetBinContent(bin) / s);
        h->SetBinError(bin, h->GetBinError(bin) / s);
      }
    }


    double TOPPhotonYieldsAlgorithm::getNominalYield(int slot, int row, int col) const
    {
      // these constants are determined from fits to MC
      const double par[8][4] = {0.0404913, -2.10514e-07, -1.01752e-08, 1.00656e-11,
                                0.0406668, -3.045e-06, -3.77908e-09, 6.4395e-12,
                                0.041261, -1.18e-06, -7.12563e-09, 8.68068e-12,
                                0.0439403, -1.38318e-06, -4.55235e-09, 6.7621e-12,
                                0.04622, 2.92406e-06, -1.4338e-08, 1.43519e-11,
                                0.0469347, 1.30621e-06, -1.10208e-08, 1.23847e-11,
                                0.0476307, 7.59058e-07, -1.1142e-08, 1.28852e-11,
                                0.0410036, 1.2299e-06, -1.26732e-08, 1.32523e-11
                               };
      const double slot_sf[16] = {1.0296, 1.00401, 0.980722, 0.997997, 1.01303, 1.00586, 0.993214, 0.978076,
                                  0.981156, 0.992147, 0.999084, 0.964364, 1.00546, 1.02583, 1.01906, 1.01039
                                 };

      if (slot < 1 or slot > 16) return 0;
      if (row < 0 or row > 7) return 0;

      double x = col - 31.5;
      x *= x;
      const double* p = par[row];

      return (p[0] + p[1] * x + p[2] * x * x + p[3] * x * x * x) * slot_sf[slot - 1];
    }


    double TOPPhotonYieldsAlgorithm::getMuonCorrection(const TH1F* h_mu, int row) const
    {
      // these constants are determined from MC
      const double numFot[8][100] = {2.35, 3.41, 5.22, 5.57, 4.26, 4.47, 6.13, 5.01, 4.93, 6.35, 5.07, 5.71, 4.88, 4.98, 4.56, 4.06,
                                     3.86, 3.38, 2.69, 2.11, 1.8, 1.44, 0.99, 0.52, 0.32, 0.26, 0.27, 0.28, 0.29, 0.36, 0.63, 1.0,
                                     1.29, 1.53, 1.89, 2.38, 2.76, 3.05, 3.34, 3.63, 3.79, 4.04, 4.2, 4.33, 4.3, 4.13, 4.0, 3.86,
                                     3.72, 3.62, 3.39, 3.14, 2.89, 2.68, 2.54, 2.44, 2.37, 2.31, 2.2, 2.09, 1.89, 1.88, 1.82, 1.76,
                                     1.7, 1.71, 1.71, 1.74, 1.7, 1.73, 1.67, 1.62, 1.49, 1.45, 1.44, 1.45, 1.36, 1.38, 1.35, 1.4,
                                     1.42, 1.46, 1.45, 1.47, 1.36, 1.24, 1.24, 1.34, 1.35, 1.33, 1.27, 1.29, 1.38, 1.53, 1.69, 1.75,
                                     1.77, 1.78, 1.75, 0.66,
                                     2.41, 3.37, 5.34, 4.54, 3.28, 4.59, 4.64, 3.62, 4.89, 4.22, 4.52, 4.75, 4.94, 4.58, 4.28, 4.21,
                                     3.54, 3.09, 2.82, 2.63, 2.46, 2.17, 1.92, 1.54, 1.07, 0.52, 0.35, 0.41, 0.66, 1.05, 1.35, 1.6,
                                     1.82, 1.99, 2.12, 2.32, 2.57, 2.98, 3.25, 3.48, 3.59, 3.65, 3.55, 3.39, 3.3, 3.23, 3.2, 3.17,
                                     3.18, 3.19, 3.17, 3.19, 3.17, 3.06, 2.84, 2.68, 2.5, 2.39, 2.25, 2.15, 1.97, 1.93, 1.89, 1.77,
                                     1.71, 1.73, 1.73, 1.77, 1.74, 1.72, 1.68, 1.63, 1.54, 1.51, 1.51, 1.46, 1.38, 1.38, 1.4, 1.41,
                                     1.45, 1.49, 1.49, 1.55, 1.46, 1.33, 1.36, 1.41, 1.49, 1.51, 1.53, 1.72, 2.04, 2.42, 2.69, 3.01,
                                     3.13, 3.13, 3.02, 1.37,
                                     2.61, 3.9, 4.86, 3.35, 3.08, 4.39, 3.66, 3.21, 4.28, 3.04, 3.95, 3.33, 3.71, 3.47, 3.49, 3.4,
                                     3.1, 2.95, 2.95, 2.88, 2.69, 2.54, 2.39, 2.22, 1.97, 1.75, 1.49, 1.38, 1.47, 1.64, 1.79, 1.91,
                                     2.03, 2.13, 2.23, 2.31, 2.39, 2.47, 2.61, 2.74, 2.7, 2.64, 2.67, 2.65, 2.66, 2.74, 2.74, 2.77,
                                     2.81, 2.81, 2.88, 2.93, 2.97, 3.01, 3.11, 3.14, 3.1, 2.97, 2.8, 2.54, 2.25, 2.23, 2.12, 2.01,
                                     1.97, 1.93, 1.91, 1.91, 1.88, 1.87, 1.86, 1.83, 1.7, 1.63, 1.62, 1.63, 1.54, 1.56, 1.55, 1.6,
                                     1.64, 1.75, 1.8, 1.96, 2.08, 2.26, 2.38, 2.59, 2.79, 2.91, 2.99, 3.14, 3.52, 3.73, 3.95, 4.1,
                                     4.13, 4.01, 3.86, 1.78,
                                     3.16, 4.53, 3.66, 2.69, 3.28, 3.9, 3.01, 3.16, 3.17, 2.91, 3.2, 2.18, 2.72, 2.11, 2.32, 2.58,
                                     2.89, 3.11, 3.31, 3.25, 3.16, 3.02, 2.89, 2.87, 3.07, 3.27, 3.27, 3.18, 2.75, 2.37, 2.22, 2.26,
                                     2.35, 2.42, 2.5, 2.53, 2.36, 2.12, 1.8, 1.63, 1.65, 1.8, 1.99, 2.06, 2.21, 2.24, 2.36, 2.45,
                                     2.51, 2.55, 2.64, 2.7, 2.77, 2.83, 2.88, 2.97, 3.08, 3.16, 3.24, 3.22, 3.15, 2.94, 2.73, 2.51,
                                     2.38, 2.34, 2.27, 2.2, 2.22, 2.16, 2.15, 2.07, 2.02, 1.94, 1.89, 1.94, 1.95, 2.0, 2.2, 2.45,
                                     2.76, 3.01, 3.28, 3.55, 3.54, 3.71, 3.68, 3.88, 3.96, 4.05, 4.08, 4.26, 4.47, 4.7, 4.74, 4.88,
                                     4.86, 4.81, 4.52, 2.24,
                                     4.53, 2.88, 1.1, 2.47, 3.57, 2.03, 1.38, 2.58, 1.56, 1.4, 1.42, 1.52, 1.54, 1.64, 1.76, 1.85,
                                     1.96, 2.12, 2.29, 2.57, 2.98, 3.48, 3.95, 4.43, 4.82, 4.95, 5.01, 4.85, 4.75, 4.21, 3.53, 2.93,
                                     2.45, 2.02, 1.82, 1.6, 1.45, 1.29, 1.19, 1.07, 0.92, 0.84, 0.77, 0.73, 0.81, 1.04, 1.33, 1.55,
                                     1.76, 1.88, 2.05, 2.13, 2.2, 2.35, 2.37, 2.49, 2.55, 2.67, 2.76, 2.9, 2.96, 3.05, 3.1, 3.16,
                                     3.34, 3.34, 3.46, 3.49, 3.69, 3.69, 3.83, 3.91, 3.99, 4.14, 4.14, 4.22, 4.37, 4.42, 4.55, 4.55,
                                     4.74, 4.9, 5.06, 5.11, 5.27, 5.28, 5.34, 5.34, 5.37, 5.56, 5.59, 5.68, 5.78, 5.87, 5.96, 6.22,
                                     6.29, 6.11, 5.57, 2.84,
                                     3.6, 1.42, 0.89, 2.24, 2.72, 1.27, 1.14, 1.17, 1.22, 1.23, 1.34, 1.32, 1.44, 1.49, 1.59, 1.71,
                                     1.79, 1.93, 2.11, 2.32, 2.58, 2.9, 3.46, 4.3, 5.16, 5.49, 5.58, 5.52, 5.22, 4.48, 3.44, 2.69,
                                     2.18, 1.86, 1.6, 1.39, 1.2, 1.03, 0.92, 0.82, 0.73, 0.65, 0.62, 0.57, 0.54, 0.52, 0.54, 0.73,
                                     1.04, 1.34, 1.55, 1.7, 1.84, 1.98, 2.04, 2.2, 2.31, 2.39, 2.51, 2.61, 2.71, 2.79, 2.93, 3.12,
                                     3.42, 3.78, 4.14, 4.5, 4.77, 5.02, 5.23, 5.49, 5.58, 5.61, 5.56, 5.55, 5.44, 5.44, 5.42, 5.49,
                                     5.54, 5.63, 5.76, 5.94, 5.99, 6.04, 5.96, 6.05, 6.15, 6.15, 6.23, 6.32, 6.42, 6.44, 6.6, 6.83,
                                     6.93, 6.9, 6.15, 3.12,
                                     2.05, 0.84, 0.9, 1.14, 1.09, 0.99, 1.08, 1.12, 1.15, 1.21, 1.23, 1.3, 1.36, 1.42, 1.53, 1.58,
                                     1.68, 1.84, 2.07, 2.3, 2.59, 2.87, 3.33, 3.96, 4.66, 5.27, 5.7, 5.58, 5.04, 4.32, 3.37, 2.71,
                                     2.29, 1.9, 1.54, 1.25, 1.0, 0.86, 0.75, 0.68, 0.61, 0.55, 0.53, 0.5, 0.47, 0.45, 0.44, 0.44,
                                     0.47, 0.57, 0.76, 1.04, 1.29, 1.55, 1.7, 1.82, 1.94, 2.1, 2.23, 2.49, 2.74, 3.14, 3.43, 3.86,
                                     4.12, 4.48, 4.74, 4.91, 5.16, 5.35, 5.57, 5.79, 5.99, 6.16, 6.33, 6.49, 6.61, 6.69, 6.75, 6.72,
                                     6.7, 6.69, 6.71, 6.64, 6.73, 6.79, 6.89, 6.75, 6.85, 6.86, 6.99, 6.98, 7.06, 7.03, 7.08, 7.35,
                                     7.44, 7.31, 6.48, 3.32,
                                     0.54, 0.54, 0.66, 0.66, 0.68, 0.77, 0.79, 0.82, 0.92, 0.9, 0.93, 1.0, 1.01, 1.07, 1.13, 1.19,
                                     1.25, 1.42, 1.54, 1.78, 2.0, 2.31, 2.7, 3.22, 3.6, 3.68, 3.64, 3.68, 3.78, 3.6, 2.83, 2.27,
                                     1.83, 1.43, 1.1, 0.89, 0.72, 0.58, 0.49, 0.45, 0.43, 0.39, 0.37, 0.33, 0.34, 0.32, 0.32, 0.31,
                                     0.34, 0.32, 0.33, 0.36, 0.43, 0.62, 0.83, 1.07, 1.37, 1.74, 2.14, 2.55, 2.88, 3.24, 3.47, 3.74,
                                     4.01, 4.29, 4.52, 4.74, 4.91, 5.1, 5.31, 5.46, 5.62, 5.84, 5.97, 6.14, 6.26, 6.41, 6.52, 6.62,
                                     6.78, 6.82, 6.98, 7.03, 7.0, 6.99, 7.0, 6.84, 6.85, 6.65, 6.59, 6.49, 6.44, 6.31, 6.12, 6.05,
                                     6.05, 6.01, 5.23, 2.58
                                    };
      const double nfotMC[8] = {2.5503, 2.5504, 2.6065, 2.7892, 2.9729, 3.0088, 3.045, 2.6138};

      if (row < 0 or row > 7) return 1;

      const double* nFot = numFot[row];
      double nfot = 0;
      for (int i = 0; i < 100; i++) {
        nfot += nFot[i] * h_mu->GetBinContent(i + 1);
      }
      nfot /= h_mu->Integral();

      return nfot / nfotMC[row];
    }



  } // end namespace TOP
} // end namespace Belle2
