/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//basf2
#include <arich/dbobjects/ARICHGeoCablesEnvelope.h>
#include <framework/logging/Logger.h>
#include <iostream>

//root
#include <TVector3.h>

using namespace std;
using namespace Belle2;

void ARICHGeoCablesEnvelope::print(const std::string& title) const
{

  ARICHGeoBase::print(title);

  cout << "Outer radius of cables envelop            : " << getEnvelopeOuterRadius() << endl
       << "Inner radius of cables envelop            : " << getEnvelopeInnerRadius() << endl
       << "Thickness of cables envelop               : " << getEnvelopeThickness() << endl
       << "Effective material name describing cables : " << getCablesEffectiveMaterialName() << endl;
  cout << "X0 of ARICH cables envelop center         : " << getEnvelopeCenterPosition().X() << endl
       << "Y0 of ARICH cables envelop center         : " << getEnvelopeCenterPosition().Y() << endl
       << "Z0 of ARICH cables envelop center         : " << getEnvelopeCenterPosition().Z() << endl;

}

void ARICHGeoCablesEnvelope::checkCablesEnvelopDataConsistency() const
{

  B2ASSERT("Data of the ARICH cables envelop is inconsisten : getEnvelopeThickness() > 0 ", getEnvelopeThickness() > 0);
  B2ASSERT("Data of the ARICH cables envelop is inconsisten : getEnvelopeOuterRadius() > getEnvelopeInnerRadius() ",
           getEnvelopeOuterRadius() > getEnvelopeInnerRadius());

}
