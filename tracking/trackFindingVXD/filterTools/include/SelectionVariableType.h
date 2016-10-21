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


#include <framework/logging/Logger.h>

#include <string>
#include <map>


namespace Belle2 {

  /** The factory serves as an interface between all selectionVariables and a user only knowing their name (in string), but not their type */
  class SelectionVariableType {
  public:
    /** allows classifying SelectionVariables */
    enum Type {
      UnknownType, // defines unknown types
      /// base type:
      SelectionVariable,
      /// SelectionVariables using 2 hits as an input:
      Distance3DSquared,
      Distance2DXYSquared,
      Distance1DZ,
      SlopeRZ,
      Distance3DNormed,
      Angle3DSimpleHighOccupancy,
      Angle3DFullHighOccupancy,
      CosAngleXYHighOccupancy,
      AngleXYFullHighOccupancy,
      AngleRZSimpleHighOccupancy,
      AngleRZFullHighOccupancy,
      CircleDist2IPHighOccupancy,
      DeltaSlopeRZHighOccupancy,
      DeltaSlopeZoverSHighOccupancy,
      DeltaSoverZHighOccupancy,
      HelixParameterFitHighOccupancy,
      PtHighOccupancy,
      CircleRadiusHighOccupancy,
      /// SelectionVariables using 3 hits as an input:
      Angle3DSimple,
      Angle3DFull,
      CosAngleXY,
      AngleXYFull,
      AngleRZSimple,
      AngleRZFull,
      CircleDist2IP,
      DeltaSlopeRZ,
      DeltaSlopeZoverS,
      DeltaSoverZ,
      HelixParameterFit,
      Pt,
      CircleRadius,
      DeltaPtHighOccupancy,
      DeltaDistCircleCenterHighOccupancy,
      DeltaCircleRadiusHighOccupancy,
      /// SelectionVariables using 4 hits as an input:
      DeltaPt,
      DeltaDistCircleCenter,
      DeltaCircleRadius,
      NTypes // number of SelectionVariables available
    };

    /** checks if a type given is a valid type for an SelectionVariableType */
    static bool isValidType(SelectionVariableType::Type type)
    {
      return ((SelectionVariableType::s_fromTypeToString.find(type) == SelectionVariableType::s_fromTypeToString.end()) ? false : true);
    }


    /** checks if the name given is a valid name for an SelectionVariableType */
    static bool isValidName(std::string type)
    {
      return ((SelectionVariableType::s_fromStringToType.find(type) == SelectionVariableType::s_fromStringToType.end()) ? false : true);
    }


    /** for given SelectionVariableType the corresponding string-name will be returned. For invalid types, UnknownType will be passed */
    static std::string getTypeName(SelectionVariableType::Type type)
    {
      auto pos = SelectionVariableType::s_fromTypeToString.find(type);
      if (pos == SelectionVariableType::s_fromTypeToString.end()) {
        B2ERROR("SelectionVariableType::getTypeName(): given iD " << type <<
                " is not a valid SelectionVariableType, return SelectionVariableType::UnknownType!");
        return SelectionVariableType::s_fromTypeToString[UnknownType];
      }
      return pos->second;
    }


    /** for given string name of a SelectionVariableType the corresponding SelectionVariableType will be returned. For invalid types, UnknownType will be passed */
    static SelectionVariableType::Type getTypeEnum(std::string type)
    {
      auto pos = SelectionVariableType::s_fromStringToType.find(type);
      if (pos == SelectionVariableType::s_fromStringToType.end()) {
        B2ERROR("SelectionVariableType::getTypeName(): given iD " << type <<
                " is not a valid SelectionVariableType, return SelectionVariableType::UnknownType!");
        return SelectionVariableType::s_fromStringToType[std::string("UnknownType")];
      }
      return pos->second;
    }


    /** returns true if given SelectionVariableType-type uses 2 hits, false if not */
    static bool isValue2HitType(SelectionVariableType::Type aType)
    { return aType < SelectionVariableType::Angle3DSimple and aType > SelectionVariableType::SelectionVariable; }


    /** returns true if given SelectionVariableType-type uses 3 hits, false if not */
    static bool isValue3HitType(SelectionVariableType::Type aType)
    { return aType < SelectionVariableType::DeltaPt and aType > SelectionVariableType::Distance3DNormed; }


    /** returns true if given SelectionVariableType-type uses 4 hits, false if not */
    static bool isValue4HitType(SelectionVariableType::Type aType)
    { return aType < SelectionVariableType::NTypes and aType > SelectionVariableType::Pt; }



  protected:

    /** static map allowing translation from a given type to its name stored as a string */
    static std::map<Type, std::string> s_fromTypeToString;

    /** static map allowing translation from a given name stored as a string to its type */
    static std::map<std::string, Type> s_fromStringToType;
  };
}

