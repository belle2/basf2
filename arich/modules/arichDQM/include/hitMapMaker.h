/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Kindo Haruki                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHHITMAPMAKER_H
#define ARICHHITMAPMAKER_H

#include <framework/core/HistoModule.h>

#include <arich/modules/arichDQM/newTHs.h>

#include <arich/dataobjects/ARICHHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHPhoton.h>
#include <arich/dataobjects/ARICHLikelihood.h>

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TVector2.h>
#include <TText.h>
#include <TFile.h>

#include <vector>
#include <string>
#include <map>

namespace Belle2 {

  /**Make hit map in HAPD view (12*12 channels)
   *@param 1D hit map of all channels
   *@param Module identifier [1:420]
   *@return 2D hit map on HAPD
   */
  TH2* moduleHitMap(TH1* hitMap, int moduleID);

  /**Make chip dead/alive map in HAPD view (2*2 chips)
   *@param 1D hit map of all channels
   *@param Module identifier [1:420]
   *@return 2D dead/alive map on HAPD
   */
  TH2* moduleDeadMap(TH1* hitMap, int moduleID);

  /**Make 1D hit map of specified Merger Board
   *@param 1D hit map of all channels
   *@param Merfer board identifier [1:72]
   *@return 1D hit map of the merger board
   */
  TH1* mergerClusterHitMap1D(TH1* hitMap, int mergerID);

  /**Make display of 6 HAPDs' 2D hit map of the Merger Board
   *@param 1D hit map of all channels
   *@param Merfer board identifier [1:72]
   *@return Display of 6 HAPDs' 2D hit map
   */
  TCanvas* mergerClusterHitMap2D(TH1* hitMap, int mergerID);

  /**Make display of 70 HAPDs' 2D hit map of the sector
   *@param 1D hit map of all channels
   *@param Sector identifier [1:6]
   *@return Display of 70 HAPDs' 2D hit map
   */
  TCanvas* sectorHitMap(TH1* hitMap, int sector);

  /**Make display of 70 HAPDs' 2D dead/alive map of the sector
   *@param 1D hit map of all channels
   *@param Sector identifier [1:6]
   *@return Display of 70 HAPDs' 2D dead/alive map
   */
  TCanvas* sectorDeadMap(TH1* hitMap, int sector);

} // Belle2 namespace

#endif
