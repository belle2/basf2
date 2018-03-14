/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>
#include <framework/logging/Logger.h>

#include <typeinfo>

#include <memory>       // std::unique_ptr
#include <utility>      // std::move, std::replace
#include <string>
#include <unordered_map>

#include <functional>

namespace Belle2 {

  /** The factory,as the name implies, does not implement at all the factory paradigm.
   * The name is an historical tribute to Jakob Lettenbichler, the main contributor of
   * the code.
   * It serves as an interface for the user who knows just the std::string name of a
   * variable and is willing to use it.
   */
  template<class PointType>
  class SelectionVariableFactory {
  public:
    /** constructor where virtual IP has been passed */
    SelectionVariableFactory(double x = 0, double y = 0, double z = 0, double = 0) :
      m_virtualIP(PointType(x, y, z)) {}

    /** typedef for more readable function-type - to be used for 2-hit-selectionVariables. */
    using TwoHitFunction =
      typename Belle2::VXDTFFilters<PointType>::twoHitFilter_t::functionType;

    /** typedef for more readable function-type - to be used for 3-hit-selectionVariables. */
    using ThreeHitFunction =
      typename Belle2::VXDTFFilters<PointType>::threeHitFilter_t::functionType;

    /** typedef for more readable function-type - to be used for 4-hit-selectionVariables. */
    //    using FourHitFunction =
    //  typename Belle2::VXDTFFilters<PointType>::fourHitFilter_t::functionType;


  protected:

    /** stores the virtual interaction point for the HiOC-filters. */
    PointType m_virtualIP;
  };
}

