/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/RunMetaData.h>

using namespace std;
using namespace Belle2;

ClassImp(RunMetaData)

bool RunMetaData::operator== (const RunMetaData& runMetaData) const
{
  return ((m_energyLER     == runMetaData.getEnergyLER())    &&
          (m_energyHER     == runMetaData.getEnergyHER())    &&
          (m_crossingAngle == runMetaData.getCrossingAngle()) &&
          (m_angleLER      == runMetaData.getAngleLER())
         );
}
