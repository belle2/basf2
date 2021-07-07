/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
