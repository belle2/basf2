/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/calibration/IRSConstants.h>
#include <framework/logging/Logger.h>

#include <TFile.h>
#include <TTree.h>
#include <iostream>

using namespace std;

namespace Belle2 {
  namespace TOP {

    bool IRSConstants::updateConstants(int run, bool replace)
    {
      if (run >= m_runFrom and run < m_runTo) return true;

      m_valid = false;

      if (m_fileName.empty()) {
        B2ERROR("IRSConstants::updateConstants: file name not given");
        return false;
      }

      // open file
      TFile* file = new TFile(m_fileName.c_str());
      if (!file->IsOpen()) {
        B2ERROR("IRSConstants::updateConstants: can't open file '" <<
                m_fileName << "' for reading");
        return false;
      }

      // get validity period
      TTree* valid = (TTree*)file->Get("validity");
      if (!valid) {
        B2ERROR("IRSConstants::updateConstants: no tree 'validity' found");
        return false;
      }
      valid->SetBranchAddress("runNumber", &m_run);
      valid->SetBranchAddress("runFrom", &m_runFrom);
      valid->SetBranchAddress("runTo", &m_runTo);
      bool found = false;
      for (int i = 0; i < valid->GetEntries(); i++) {
        valid->GetEntry(i);
        found = (run >= m_runFrom and run < m_runTo);
        if (found) break;
      }
      if (!found) {
        B2ERROR("IRSConstants::updateConstants: no valid constants for run number " <<
                run << " found");
        return false;
      }

      // get tree with constants
      TTree* tree = (TTree*)file->Get("IRSConstants");
      if (!tree) {
        B2ERROR("IRSConstants::updateConstants: no tree 'IRSConstants' found");
        return false;
      }
      TBranch* branch = tree->GetBranch("channel");
      if (!branch) {
        B2ERROR("IRSConstants::updateConstants: no branch 'channel' found");
        return false;
      }

      // delete old calibration constants
      for (unsigned k = 0; k < c_NumBars; k++) {
        auto& bar = m_bar[k];
        for (auto& channel : bar) {
          if (channel) delete channel;
        }
      }

      // read-in new ones
      TObject* obj = 0;
      for (unsigned i = 0; i < tree->GetEntries(); i++) {
        obj = 0;
        branch->SetAddress(&obj);
        tree->GetEntry(i);
        auto* channelConstants = static_cast<TOP::ASICChannelConstants*>(obj);
        setConstants(channelConstants, replace);
      }

      m_valid = true;
      file->Close();
      return true;
    }


    bool IRSConstants::writeConstants(int run, int runFrom, int runTo)
    {
      if (m_fileName.empty()) {
        B2ERROR("IRSConstants::setConstants: file name not given");
        return false;
      }

      TFile* file = new TFile(m_fileName.c_str(), "RECREATE");
      if (file->IsZombie()) {
        B2ERROR("IRSConstants::setConstants: can't open file '" <<
                m_fileName << "' for writing");
        return false;
      }

      TTree* tree = new TTree("IRSConstants", "Calibration constants of IRS ASIC's");
      ASICChannelConstants* channelConstants = 0;
      tree->Branch("channel", &channelConstants);

      for (unsigned k = 0; k < c_NumBars; k++) {
        auto& bar = m_bar[k];
        for (auto& channel : bar) {
          if (!channel) continue;
          if (channel->getNumofGoodWindows() == 0) continue;
          channelConstants = channel;
          tree->Fill();
        }
      }
      tree->Write();

      m_valid = true;
      m_run = run;
      m_runFrom = runFrom;
      m_runTo = runTo;

      TTree* valid = new TTree("validity", "valid run ranges for these constants");
      valid->Branch("runNumber", &m_run);   // production run number of the constants
      valid->Branch("runFrom", &m_runFrom); // constants valid for run numbers from (incl)
      valid->Branch("runTo", &m_runTo); // constants valid for run numbers up to (excl)
      valid->Fill();
      valid->Write();

      file->Close();
      return true;
    }


    void IRSConstants::setConstants(ASICChannelConstants* constants, bool replace)
    {
      if (constants->getNumofGoodWindows() == 0) {
        delete constants;
        return;
      }

      int barID = constants->getBarID();
      if (barID < 1 or barID > c_NumBars) {
        B2ERROR("IRSConstants::setConstants: invalid bar ID " << barID);
        return;
      }
      barID--;
      auto& bar = m_bar[barID];
      unsigned channel = constants->getChannel();
      if (channel >= bar.size()) {
        B2ERROR("IRSConstants::setConstants: invalid channel number " << channel);
        return;
      }
      if (bar[channel]) {
        if (replace) {
          B2INFO("IRSConstants::setConstants: existing constants for bar "
                 << barID + 1 << " channel " << channel << " are replaced");
          delete bar[channel];
          bar[channel] = constants;
        } else {
          B2INFO("IRSConstants::setConstants: existing constants for bar "
                 << barID + 1 << " channel " << channel << " are kept");
          delete constants;
        }
      } else {
        bar[channel] = constants;
      }

    }


  } // end namespace TOP
} // end namespace Belle2






