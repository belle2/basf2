/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/calibration/DeadBoardAlgorithm.h>
#include <calibration/CalibrationAlgorithm.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/logging/Logger.h>

#include <TH1F.h>

#include <cmath>

using namespace Belle2;
using namespace CDC;

DeadBoardAlgorithm::DeadBoardAlgorithm() :
  CalibrationAlgorithm("CDCDeadBoardDetector")
{
  setDescription(
    " -------------------------- Dead Board Detection Algorithm -------------------------\n"
  );
}

void DeadBoardAlgorithm::detectDeadBoards()
{
  auto hBoardIDs = getObjectPtr<TH1F>(m_histName);

  if (!hBoardIDs) {
    B2ERROR("Histogram " << m_histName << " not found");
    return;
  }

  B2INFO("Found histogram " << m_histName
         << ", nbins = " << hBoardIDs->GetNbinsX()
         << ", entries = " << hBoardIDs->GetEntries());

  for (int ibin = 1; ibin <= hBoardIDs->GetNbinsX(); ++ibin) {
    const double content = hBoardIDs->GetBinContent(ibin);

    // Use bin center as board ID
    const int boardID = static_cast<int>(std::lround(hBoardIDs->GetXaxis()->GetBinCenter(ibin)));

    // Temporary/simple logic:
    // a board with zero (or below-threshold) entries is marked as dead.
    if (content <= m_threshold) {
      B2INFO("Dead board candidate found: boardID = " << boardID
             << ", bin = " << ibin
             << ", content = " << content);

      // eff = 0 means dead board
      m_badBoardList->setBoard(boardID, 0.0);
    }
  }

  B2INFO("Total dead boards found: " << m_badBoardList->getEntries());
  m_badBoardList->dump();
}

CalibrationAlgorithm::EResult DeadBoardAlgorithm::calibrate()
{
  m_badBoardList = new CDCBadBoards();

  const auto exprun = getRunList()[0];
  B2INFO("ExpRun used for DB Geometry : " << exprun.first << " " << exprun.second);
  updateDBObjPtrs(1, exprun.second, exprun.first);

  CDC::CDCGeometryPar::Instance(&(*m_cdcGeo));

  detectDeadBoards();

  saveCalibration(m_badBoardList, "CDCBadBoards");
  return c_OK;
}
