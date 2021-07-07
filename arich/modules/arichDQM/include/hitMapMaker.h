/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHHITMAPMAKER_H
#define ARICHHITMAPMAKER_H

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

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
