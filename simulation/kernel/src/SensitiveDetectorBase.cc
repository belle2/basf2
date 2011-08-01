/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/SensitiveDetectorBase.h>

using namespace std;
namespace Belle2 {
  namespace Simulation {
    bool SensitiveDetectorBase::m_active(false);
    set<string> SensitiveDetectorBase::m_mcRelations;
  }
}
