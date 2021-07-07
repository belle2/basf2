/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

