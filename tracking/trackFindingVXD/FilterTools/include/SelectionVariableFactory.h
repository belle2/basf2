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

// 2-hit:
#include <tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h>
#include <tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h>
#include <tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h>
#include <tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h>
#include <tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h>
// 3-hit:
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Angle3DFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleXYSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleXYFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleRZSimple.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/AngleRZFull.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/CircleDist2IP.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeRZ.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeZoverS.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/DeltaSoverZ.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/HelixParameterFit.h>
#include <tracking/trackFindingVXD/ThreeHitFilters/Pt.h>
// 4-hit:
#include <tracking/trackFindingVXD/FourHitFilters/DeltaPt.h>
#include <tracking/trackFindingVXD/FourHitFilters/DeltaDistCircleCenter.h>

#include <framework/logging/Logger.h>

#include <typeinfo>
#include <boost/regex.hpp>
#include <cxxabi.h>

#include <memory>       // std::unique_ptr
#include <utility>      // std::move
#include <string>       // std::string



namespace Belle2 {

  /** The factory serves as an interface between all selectionVariables and a user only knowing their name (in string), but not their type */
  template<class PointType>
  class SelectionVariableFactory {
  public:
    /** creates the full typeName using the easy-to-read-one as an input */
    std::string getFullVariableName(std::string variableName)
    {
      char* realname(nullptr);
      int status(0);
      auto typeName = typeid(PointType).name();
      realname = abi::__cxa_demangle(typeName, nullptr, nullptr, &status);
      std::string name(realname);
      free(realname);
      // some formatting because of root:
      boost::regex colon("(:)");
      auto name1 = boost::regex_replace(name, colon, std::string("_"));
      boost::regex lesser("(<)");
      auto name2 = boost::regex_replace(name1, lesser, std::string("{"));
      boost::regex greater("(>)");
      auto name3 = boost::regex_replace(name2, greater, std::string("}"));

      return "Belle2__" + variableName + "{" + name3 + "}";
    }


    /** WARNING does currently not work, since the value-functions of the base class are not virtual! */
    std::unique_ptr<SelectionVariable<PointType, double>> getSelectionVariable(std::string variableName)
    {
      /// 2-hit:
      if (getFullVariableName(variableName) == Distance3DSquared<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Distance3DSquared<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Distance2DXYSquared<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Distance2DXYSquared<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Distance1DZ<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Distance1DZ<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Distance1DZTemplate<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Distance1DZTemplate<PointType>())
               );
      }

      if (getFullVariableName(variableName) == SlopeRZ<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new SlopeRZ<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Distance3DNormed<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Distance3DNormed<PointType>())
               );
      }


      /// 3-hit:
      if (getFullVariableName(variableName) == Angle3DSimple<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Angle3DSimple<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Angle3DFull<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Angle3DFull<PointType>())
               );
      }

      if (getFullVariableName(variableName) == AngleXYSimple<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new AngleXYSimple<PointType>())
               );
      }

      if (getFullVariableName(variableName) == AngleXYFull<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new AngleXYFull<PointType>())
               );
      }

      if (getFullVariableName(variableName) == AngleRZSimple<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new AngleRZSimple<PointType>())
               );
      }

      if (getFullVariableName(variableName) == AngleRZFull<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new AngleRZFull<PointType>())
               );
      }

      if (getFullVariableName(variableName) == CircleDist2IP<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new CircleDist2IP<PointType>())
               );
      }

      if (getFullVariableName(variableName) == DeltaSlopeRZ<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new DeltaSlopeRZ<PointType>())
               );
      }

      if (getFullVariableName(variableName) == DeltaSlopeZoverS<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new DeltaSlopeZoverS<PointType>())
               );
      }

      if (getFullVariableName(variableName) == DeltaSoverZ<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new DeltaSoverZ<PointType>())
               );
      }

      if (getFullVariableName(variableName) == HelixParameterFit<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new HelixParameterFit<PointType>())
               );
      }

      if (getFullVariableName(variableName) == Pt<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new Pt<PointType>())
               );
      }


      /// 4-hit:
      if (getFullVariableName(variableName) == DeltaPt<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new DeltaPt<PointType>())
               );
      }

      if (getFullVariableName(variableName) == DeltaDistCircleCenter<PointType>().name()) {
        return std::move(
                 std::unique_ptr<SelectionVariable<PointType, double> >
                 (new DeltaDistCircleCenter<PointType>())
               );
      }

      B2ERROR(" SelectionVariableFactory: given name (raw/full): " << variableName <<
              "/" << getFullVariableName(variableName) <<
              " is not known, returning non-functioning base-class instead!")

      return std::move(
               std::unique_ptr<SelectionVariable<PointType, double> >
               (new SelectionVariable<PointType, double>())
             );
    }



    /** for given SelectionVariable-name and pair of hits, result is calculated if SelectionVariable exists */
    double getResult2Hit(
      std::string variableName,
      const PointType& outerHit,
      const PointType& innerHit)
    {
      /// 2-hit:
      if (getFullVariableName(variableName) == Distance3DSquared<PointType>().name())
      { return Distance3DSquared<PointType>::value(outerHit, innerHit); }

      if (getFullVariableName(variableName) == Distance2DXYSquared<PointType>().name())
      { return Distance2DXYSquared<PointType>::value(outerHit, innerHit); }

      if (getFullVariableName(variableName) == Distance1DZ<PointType>().name())
      { return Distance1DZ<PointType>::value(outerHit, innerHit); }

      if (getFullVariableName(variableName) == Distance1DZTemplate<PointType>().name())
      { return Distance1DZTemplate<PointType>::value(outerHit, innerHit); }

      if (getFullVariableName(variableName) == SlopeRZ<PointType>().name())
      { return SlopeRZ<PointType>::value(outerHit, innerHit); }

      if (getFullVariableName(variableName) == Distance3DNormed<PointType>().name())
      { return Distance3DNormed<PointType>::value(outerHit, innerHit); }

      B2ERROR(" SelectionVariableFactory-2Hit: given name (raw/full): " << variableName <<
              "/" << getFullVariableName(variableName) <<
              " is not known, returning '0' as a result instead!")

      return 0.0;
    }



    /** for given SelectionVariable-name and triplet of hits, result is calculated if SelectionVariable exists */
    double getResult3Hit(
      std::string variableName,
      const PointType& outerHit,
      const PointType& centerHit,
      const PointType& innerHit)
    {

      /// 3-hit:
      if (getFullVariableName(variableName) == Angle3DSimple<PointType>().name())
      { return Angle3DSimple<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == Angle3DFull<PointType>().name())
      { return Angle3DFull<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == AngleXYSimple<PointType>().name())
      { return AngleXYSimple<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == AngleXYFull<PointType>().name())
      { return AngleXYFull<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == AngleRZSimple<PointType>().name())
      { return AngleRZSimple<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == AngleRZFull<PointType>().name())
      { return AngleRZFull<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == CircleDist2IP<PointType>().name())
      { return CircleDist2IP<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == DeltaSlopeRZ<PointType>().name())
      { return DeltaSlopeRZ<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == DeltaSlopeZoverS<PointType>().name())
      { return DeltaSlopeZoverS<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == DeltaSoverZ<PointType>().name())
      { return DeltaSoverZ<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == HelixParameterFit<PointType>().name())
      { return HelixParameterFit<PointType>::value(outerHit, centerHit, innerHit); }

      if (getFullVariableName(variableName) == Pt<PointType>().name())
      { return Pt<PointType>::value(outerHit, centerHit, innerHit); }

      B2ERROR(" SelectionVariableFactory-3Hit: given name (raw/full): " << variableName <<
              "/" << getFullVariableName(variableName) <<
              " is not known, returning '0' as a result instead!")

      return 0.0;
    }



    /** for given SelectionVariable-name and quadruplet of hits, result is calculated if SelectionVariable exists */
    double getResult4Hit(
      std::string variableName,
      const PointType& outerHit,
      const PointType& outerCenterHit,
      const PointType& innerCenterHit,
      const PointType& innerHit)
    {

      /// 4-hit:
      if (getFullVariableName(variableName) == DeltaPt<PointType>().name())
      { return DeltaPt<PointType>::value(outerHit, outerCenterHit, innerCenterHit, innerHit); }

      if (getFullVariableName(variableName) == DeltaDistCircleCenter<PointType>().name())
      { return DeltaDistCircleCenter<PointType>::value(outerHit, outerCenterHit, innerCenterHit, innerHit); }

      B2ERROR(" SelectionVariableFactory-4Hit: given name (raw/full): " << variableName <<
              "/" << getFullVariableName(variableName) <<
              " is not known, returning '0' as a result instead!")

      return 0.0;
    }
  };

}
