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

#include <arich/modules/arichTreeMaker/newTHs.h>

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

  TH2* moduleHitMap(TH1* hitMap, int moduleID);
  TH2* moduleDeadMap(TH1* hitMap, int moduleID);
  TH1* mergerClusterHitMap1D(TH1* hitMap, int mergerID);
  TCanvas* mergerClusterHitMap2D(TH1* hitMap, int mergerID);
  TCanvas* sectorHitMap(TH1* hitMap, int sector);
  TCanvas* sectorDeadMap(TH1* hitMap, int sector);

} // Belle2 namespace

#endif
