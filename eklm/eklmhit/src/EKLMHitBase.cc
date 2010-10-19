/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMHitBase.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

//Geant4 classes
#include "G4VVisManager.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4Polyline.hh"
#include "G4Circle.hh"


using namespace Belle2;

ClassImp(EKLMHitBase);

EKLMHitBase::EKLMHitBase(const char * name)
{
  m_Name = name;
}
EKLMHitBase::EKLMHitBase(std::string &name)
{
  m_Name = name;
}


void EKLMHitBase::Print()
{
  INFO("Not implemented yet");
}

void EKLMHitBase::Draw()
{
  // buggy

  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if (pVVisManager) {
    G4Colour colour(1., 1., 1.);
    G4VisAttributes attribs(colour);
    G4Circle circle;
    //    circle.SetPosition(G4Point3D(m_hitPosition.x(), m_hitPosition.y(), m_hitPosition.z()));
    circle.SetScreenDiameter(2.0);
    circle.SetFillStyle(G4Circle::filled);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

