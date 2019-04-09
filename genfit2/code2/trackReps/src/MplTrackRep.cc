/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "MplTrackRep.h"

#include <framework/logging/Logger.h>

using namespace genfit;

MplTrackRep::MplTrackRep(int pdgCode, float magCharge, char propDir) :
  RKTrackRep(pdgCode, propDir),
  m_magCharge(magCharge)
{
  B2RESULT("Monopole representation is created");
}

MplTrackRep::~MplTrackRep()
{
}

double MplTrackRep::RKPropagate(M1x7& state7,
                                M7x7* jacobian,
                                M1x3& SA,
                                double S,
                                bool varField,
                                bool calcOnlyLastRowOfJ) const
{
  B2RESULT("Propagation with monopole representation is called");
  return 0.0;
}
