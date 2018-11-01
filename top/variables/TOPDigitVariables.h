/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <top/dataobjects/TOPDigit.h>

using namespace std;

namespace Belle2 {
  namespace TOPDigitVariables {
  }
  namespace Variable {
    /**
     * returns the number of digits in the module hit by the particle
     */
    double TOPModuleDigitCount(const Particle*);

    /**
     * returns the largest time difference between two consecutive hits in the module
     */
    double TOPModuleDigitGapSize(const Particle*);

    /**
     * returns the number of reflected photons in the module
     */
    double TOPModuleReflectedDigitCount(const Particle*);
  }
}
