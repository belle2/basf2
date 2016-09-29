/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <tracking/trackFindingVXD/filterTools/SelectionVariableType.h>

// 2-hit:
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/Distance3DSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/Distance1DZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/Distance1DZTemp.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/SlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitFilters/Distance3DNormed.h>
// 3-hit:
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/Angle3DSimple.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/Angle3DFull.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/AngleXYSimple.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/AngleXYFull.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/AngleRZSimple.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/AngleRZFull.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/CircleDist2IP.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/DeltaSlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/DeltaSlopeZoverS.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/DeltaSoverZ.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/HelixParameterFit.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/Pt.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitFilters/CircleRadius.h>
// 4-hit:
#include <tracking/trackFindingVXD/sectorMap/fourHitFilters/DeltaPt.h>
#include <tracking/trackFindingVXD/sectorMap/fourHitFilters/DeltaDistCircleCenter.h>
#include <tracking/trackFindingVXD/sectorMap/fourHitFilters/DeltaCircleRadius.h>

#include <framework/logging/Logger.h>

#include <typeinfo>
#include <cxxabi.h>

#include <memory>       // std::unique_ptr
#include <utility>      // std::move, std::replace
#include <string>
#include <unordered_map>

#include <functional>

namespace Belle2 {

  /** The factory serves as an interface between all selectionVariables and a user only knowing their name (in string), but not their type. */
  template<class PointType>
  class SelectionVariableFactory {
  public:
    /** constructor where virtual IP has been passed */
    SelectionVariableFactory(double x = 0, double y = 0, double z = 0, double = 0) :
      m_virtualIP(PointType(x, y, z)) {}



    /** shortCut for better readability. */
    typedef SelectionVariableType::Type SelVarType;

    /** typedef for more readable function-type - to be used for 2-hit-selectionVariables. */
    using TwoHitFunction = typename std::function<double(const PointType&, const PointType&)>;

    /** typedef for more readable function-type - to be used for 3-hit-selectionVariables. */
    using ThreeHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&)>;

    /** typedef for more readable function-type - to be used for 4-hit-selectionVariables. */
    using FourHitFunction = typename std::function<double(const PointType&, const PointType&, const PointType&, const PointType&)>;



    /** For given name of a variableType a function for the corresponding SelectionVariable is returned. */
    TwoHitFunction get2HitInterface(std::string variableName)
    {
      SelVarType varType = SelectionVariableType::getTypeEnum(variableName);
      return get2HitInterface(varType);
    }



    /** For given variableType a function for the corresponding SelectionVariable is returned. */
    TwoHitFunction get2HitInterface(SelVarType variableType)
    {
      if (variableType == SelectionVariableType::Distance3DSquared) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Distance3DSquared<PointType>::value(outerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Distance2DXYSquared) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Distance2DXYSquared<PointType>::value(outerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Distance1DZ) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Distance1DZ<PointType>::value(outerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Distance1DZTemplate) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Distance1DZTemplate<PointType>::value(outerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::SlopeRZ) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return SlopeRZ<PointType>::value(outerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Distance3DNormed) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Distance3DNormed<PointType>::value(outerHit, innerHit); };
      }


      // 2+1 hits:

      if (variableType == SelectionVariableType::Angle3DSimpleHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Angle3DSimple<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::Angle3DFullHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Angle3DFull<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::AngleXYSimpleHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return AngleXYSimple<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::AngleXYFullHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return AngleXYFull<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::AngleRZSimpleHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return AngleRZSimple<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::AngleRZFullHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return AngleRZFull<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::CircleDist2IPHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return CircleDist2IP<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::DeltaSlopeRZHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return DeltaSlopeRZ<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::DeltaSlopeZoverSHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return DeltaSlopeZoverS<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::DeltaSoverZHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return DeltaSoverZ<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::HelixParameterFitHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return HelixParameterFit<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::PtHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return Pt<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::CircleRadiusHighOccupancy) {
        return [ & ](const PointType & outerHit, const PointType & innerHit) -> double
        { return CircleRadius<PointType>::value(outerHit, innerHit, m_virtualIP); };
      }


      B2ERROR(" SelectionVariableFactory-2Hit: given name (raw/full): " << variableType <<
              "/" << SelectionVariableType::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as return value!");

      return [&](const PointType&, const PointType&) -> double { return 0.0; };
    }




    /** For given name of a variableType a function for the corresponding SelectionVariable is returned. */
    ThreeHitFunction get3HitInterface(std::string variableName)
    {
      SelVarType varType = SelectionVariableType::getTypeEnum(variableName);
      return get3HitInterface(varType);
    }



    /** For given variableType a function for the corresponding SelectionVariable is returned. */
    ThreeHitFunction get3HitInterface(SelVarType variableType)
    {
      if (variableType == SelectionVariableType::Angle3DSimple) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return Angle3DSimple<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Angle3DFull) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return Angle3DFull<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::AngleXYSimple) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return AngleXYSimple<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::AngleXYFull) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return AngleXYFull<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::AngleRZSimple) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return AngleRZSimple<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::AngleRZFull) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return AngleRZFull<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::CircleDist2IP) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return CircleDist2IP<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::DeltaSlopeRZ) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaSlopeRZ<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::DeltaSlopeZoverS) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaSlopeZoverS<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::DeltaSoverZ) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaSoverZ<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::HelixParameterFit) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return HelixParameterFit<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::Pt) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return Pt<PointType>::value(outerHit, centerHit, innerHit); };
      }

      if (variableType == SelectionVariableType::CircleRadius) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return CircleRadius<PointType>::value(outerHit, centerHit, innerHit); };
      }


      // 3+1 hits:

      if (variableType == SelectionVariableType::DeltaPtHighOccupancy) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaPt<PointType>::value(outerHit, centerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::DeltaDistCircleCenterHighOccupancy) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaDistCircleCenter<PointType>::value(outerHit, centerHit, innerHit, m_virtualIP); };
      }

      if (variableType == SelectionVariableType::DeltaCircleRadiusHighOccupancy) {
        return [ & ](const PointType & outerHit,
                     const PointType & centerHit,
                     const PointType & innerHit) -> double
        { return DeltaCircleRadius<PointType>::value(outerHit, centerHit, innerHit, m_virtualIP); };
      }
      B2ERROR(" SelectionVariableFactory-3Hit: given name (raw/full): " << variableType <<
              "/" << SelectionVariableType::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as return value!");

      return [&](const PointType&, const PointType&, const PointType&) -> double { return 0.0; };
    }



    /** For given name of a variableType a function for the corresponding SelectionVariable is returned. */
    FourHitFunction get4HitInterface(std::string variableName)
    {
      SelVarType varType = SelectionVariableType::getTypeEnum(variableName);
      return get4HitInterface(varType);
    }



    /** For given variableType a function for the corresponding SelectionVariable is returned. */
    FourHitFunction get4HitInterface(SelVarType variableType)
    {
      if (variableType == SelectionVariableType::DeltaPt) {
        return [ & ](const PointType & outerHit,
                     const PointType & outerCenterHit,
                     const PointType & innerCenterHit,
                     const PointType & innerHit) -> double
        { return DeltaPt<PointType>::value(outerHit, outerCenterHit, innerCenterHit, innerHit); };
      }

      if (variableType == SelectionVariableType::DeltaDistCircleCenter) {
        return [ & ](const PointType & outerHit,
                     const PointType & outerCenterHit,
                     const PointType & innerCenterHit,
                     const PointType & innerHit) -> double
        { return DeltaDistCircleCenter<PointType>::value(outerHit, outerCenterHit, innerCenterHit, innerHit); };
      }

      if (variableType == SelectionVariableType::DeltaCircleRadius) {
        return [ & ](const PointType & outerHit,
                     const PointType & outerCenterHit,
                     const PointType & innerCenterHit,
                     const PointType & innerHit) -> double
        { return DeltaCircleRadius<PointType>::value(outerHit, outerCenterHit, innerCenterHit, innerHit); };
      }


      B2ERROR(" SelectionVariableFactory-4Hit: given name (raw/full): " << variableType <<
              "/" << SelectionVariableType::getTypeName(variableType) <<
              " is not known, returning dummy function with 0.0 as return value!");

      return [&](const PointType&, const PointType&, const PointType&, const PointType&) -> double { return 0.0; };
    }


  protected:

    /** stores the virtual interaction point for the HiOC-filters. */
    PointType m_virtualIP;
  };
}

