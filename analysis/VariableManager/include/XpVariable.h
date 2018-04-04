/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
//#include <framework/gearbox/Const.h>
#include <analysis/VariableManager/Manager.h>


namespace Belle2 {

  namespace Variable {
    /**
    * return scaled mometnum
    *
    */
    double particleXp(const Particle* part);
  };
}
