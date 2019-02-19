/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "TEveManager.h"
#include "TEveGeoNode.h"

#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoVolume.h"
#include "TGeoMedium.h"

void EveGeo()
{
  TEveManager::Create();

  gGeoManager = gEve->GetGeometry("Belle2Geo.root");

  TGeoNode* node = gGeoManager->GetTopNode();
  TEveGeoTopNode* its = new TEveGeoTopNode(gGeoManager, node);
  gEve->AddGlobalElement(its);

  its->SetVisOption(0);
  its->SetVisLevel(5);

  gEve->Redraw3D(kTRUE);
}

