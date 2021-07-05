/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPChannelMaskAlgorithm.h>
#include <framework/logging/Logger.h>
#include "TROOT.h"
#include <TH1F.h>
#include <TH2F.h>
#include <string>
#include <vector>

using namespace std;

namespace Belle2 {
  namespace TOP {

    TOPChannelMaskAlgorithm::TOPChannelMaskAlgorithm():
      CalibrationAlgorithm("TOPChannelMaskCollector")
    {
      setDescription("Calibration algorithm for masking of dead and hot channels");
    }

    CalibrationAlgorithm::EResult TOPChannelMaskAlgorithm::calibrate()
    {
      gROOT->SetBatch();

      // check if the statistics is sufficient

      auto nhits = getObjectPtr<TH1F>("nhits");
      if (not nhits) return c_NotEnoughData;
      double averageChannelHits = nhits->GetEntries() * nhits->GetMean() / 16 / 512;
      B2INFO("Average number of good hits per channel: " << averageChannelHits);
      if (averageChannelHits < m_minHits) return c_NotEnoughData;

      // construct file name and open output root file

      const auto& expRun = getRunList();
      string expNo = to_string(expRun[0].first);
      while (expNo.length() < 4) expNo.insert(0, "0");
      string runNo = to_string(expRun[0].second);
      while (runNo.length() < 5) runNo.insert(0, "0");
      string outputFileName = "channelMask-e" + expNo + "-r" + runNo + ".root";
      m_file = TFile::Open(outputFileName.c_str(), "recreate");

      nhits->Write();

      // create payload

      m_channelMask = new TOPCalChannelMask();

      // dead and hot channels

      auto meanHits = new TH1F("meanHits", "Average number of hits per channel; slot number; average", 16, 0.5, 16.5);
      for (int slot = 1; slot <= 16; slot++) {
        string name = "hits_" + to_string(slot);
        auto h = getObjectPtr<TH1F>(name);
        if (not h) continue;
        h->Write();
        double mean = 0;
        int n = 0;
        for (int chan = 0; chan < h->GetNbinsX(); chan++) {
          double y = h->GetBinContent(chan + 1);
          if (y > 0) {
            mean += y;
            n++;
          }
        }
        if (n > 0) mean /= n;
        meanHits->SetBinContent(slot, mean);
        double deadCut = mean / 10;
        double hotCut = mean * 10;
        for (int chan = 0; chan < h->GetNbinsX(); chan++) {
          double y = h->GetBinContent(chan + 1);
          if (y <= deadCut) {
            m_channelMask->setDead(slot, chan);
          } else if (y > hotCut) {
            m_channelMask->setNoisy(slot, chan);
          }
        }
      }

      // ASICs with window corruption

      for (int slot = 1; slot <= 16; slot++) {
        string name = "window_vs_asic_" + to_string(slot);
        auto h = getObjectPtr<TH2F>(name);
        if (not h) continue;
        h->Write();
        auto h0 = h->ProjectionX();
        auto h1 = h->ProjectionX("_tmp", m_minWindow, m_maxWindow);
        for (int asic = 0; asic < h->GetNbinsX(); asic++) {
          double r = 1 - h1->GetBinContent(asic + 1) / h0->GetBinContent(asic + 1);
          if (r > 0.20) {
            for (int chan = 0; chan < 8; chan++) m_channelMask->setNoisy(slot, chan + asic * 8);
          }
        }
        delete h0;
        delete h1;
      }

      // write the results and close the file

      auto dead = new TH1F("numDead", "Number of dead channels; slot number; dead channels", 16, 0.5, 16.5);
      auto hot = new TH1F("numHot", "Number of noisy channels; slot number; noisy channels", 16, 0.5, 16.5);
      auto active = new TH1F("activeFract", "Fraction of active channels; slot number; active fraction", 16, 0.5, 16.5);
      for (int slot = 1; slot <= 16; slot++) {
        dead->SetBinContent(slot, m_channelMask->getNumOfDeadChannels(slot));
        hot->SetBinContent(slot, m_channelMask->getNumOfNoisyChannels(slot));
        active->SetBinContent(slot, m_channelMask->getActiveFraction(slot));
      }
      m_file->Write();
      m_file->Close();

      // import payload to DB

      saveCalibration(m_channelMask);

      return c_OK;
    }


  } // end namespace TOP
} // end namespace Belle2

