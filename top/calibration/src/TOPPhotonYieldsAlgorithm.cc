/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPPhotonYieldsAlgorithm.h>
#include <top/dbobjects/TOPCalPhotonYields.h>
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
      setDescription("Calibration algorithm for photon pixel yields aimed for PMT ageing studies and for finding optically decoupled PMT's");
    }

    CalibrationAlgorithm::EResult TOPPhotonYieldsAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // construct file name and open output root file for storing merged histograms

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "photonYields-e" + expNo + "-r" + runNo + ".root";
      auto* file = TFile::Open(outputFileName.c_str(), "recreate");

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

      // construct DB object for storing the results and set the time stamp

      auto* dbobject = new TOPCalPhotonYields();
      dbobject->setTimeStamp(timeStamp->GetBinContent(1), timeStamp->GetBinError(1));

      // for each slot determine photon pixel yields and equalized alpha ratio and store them into DB object

      int numModules = numTracks->GetNbinsX();
      for (int slot = 1; slot <= numModules; slot++) {
        string slotName = (slot < 10) ? "_0" + to_string(slot) : "_" + to_string(slot);

        auto signalHits = getObjectPtr<TH1F>("signalHits" + slotName);
        if (not signalHits) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'signalHits' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        signalHits->Write();

        auto bkgHits = getObjectPtr<TH1F>("bkgHits" + slotName);
        if (not bkgHits) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'bkgHits' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        bkgHits->Write();

        auto activePixels = getObjectPtr<TH1F>("activePixels" + slotName);
        if (not activePixels) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'activePixels' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        activePixels->Write();

        auto alphaLow = getObjectPtr<TH1F>("alphaLow" + slotName);
        if (not alphaLow) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'alphaLow' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        alphaLow->Sumw2();
        alphaLow->Write();

        auto alphaHigh = getObjectPtr<TH1F>("alphaHigh" + slotName);
        if (not alphaHigh) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'alphaHigh' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        alphaHigh->Sumw2();
        alphaHigh->Write();

        auto pulseHeights = getObjectPtr<TH2F>("pulseHeights" + slotName);
        if (not pulseHeights) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'pulseHeights' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
          return c_Failure;
        }
        pulseHeights->Write();

        auto muonZ = getObjectPtr<TH1F>("muonZ" + slotName);
        if (not muonZ) {
          B2ERROR("TOPPhotonYieldsAlgorithm: histogram 'muonZ' for slot " << slot << " not found");
          file->Close();
          delete dbobject;
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

        dbobject->set(slot, photonYields, bkgYields, alphaRatio, activePixels.get(), pulseHeights.get(), muonZ.get());

        delete photonYields;
        delete bkgYields;
        delete alphaRatio;
      }

      file->Close();

      saveCalibration(dbobject);

      return c_OK;
    }


    double TOPPhotonYieldsAlgorithm::getEqualizingValue(int bin)
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


    void TOPPhotonYieldsAlgorithm::equalize(TH1F* h)
    {
      for (int bin = 1; bin <= h->GetNbinsX(); bin++) {
        double s = getEqualizingValue(bin);
        h->SetBinContent(bin, h->GetBinContent(bin) / s);
        h->SetBinError(bin, h->GetBinError(bin) / s);
      }
    }

  } // end namespace TOP
} // end namespace Belle2
