/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <map>
#include <string>

namespace Belle2 {
  namespace SoftwareTrigger {
    /**
     * Base object to store the variable_name to double value map,
     * which is used in the software trigger variable manager and the cuts.
     * This map has to be compiled before using any cut and has to be given
     * to the cut whenever it is checked.
     *
     * This has the advantage that the values are only created once and can
     * share temporary objects during calculation.
     */
    typedef std::map<std::string, double> SoftwareTriggerObject;
  }
}
