/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// framework:
#include <framework/logging/Logger.h>

// stl:
#include <string>
#include <map>



namespace Belle2 {
  /** Small class for classifying types of analyzing algorithms. Contains enums and converter from and to strings */
  class AlgoritmType {
  public:

    /** allows classifying Analyzing algorithms */
    enum Type {
      UnknownType, // defines unknown types
      /// base type, template for all the other analyzing algorithms, contains essential functions for all algorithms:
      AnalyzingAlgorithmBase,
      /// value type, stores double. defined in ..tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmValues.h:
      AnalyzingAlgorithmValuePX,
      AnalyzingAlgorithmValuePY,
      AnalyzingAlgorithmValuePZ,
      AnalyzingAlgorithmValuePT,
      AnalyzingAlgorithmValueP,
      AnalyzingAlgorithmValuePTheta,
      AnalyzingAlgorithmValuePPhi,
      AnalyzingAlgorithmValueDistSeed2IP,
      AnalyzingAlgorithmValueDistSeed2IPZ,
      AnalyzingAlgorithmValueDistSeed2IPXY,
      AnalyzingAlgorithmValueQI,
      /// residual type, stores double. defined in ..tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmResiduals.h:
      AnalyzingAlgorithmResidualPX,
      AnalyzingAlgorithmResidualPY,
      AnalyzingAlgorithmResidualPZ,
      AnalyzingAlgorithmResidualPT,
      AnalyzingAlgorithmResidualP,
      AnalyzingAlgorithmResidualPTheta,
      AnalyzingAlgorithmResidualPPhi,
      AnalyzingAlgorithmResidualPAngle,
      AnalyzingAlgorithmResidualPTAngle,
      AnalyzingAlgorithmResidualPosition,
      AnalyzingAlgorithmResidualPositionXY,
      /// value type hit (with access to clusters via relations), stores int. defined in ..tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmClusterBased.h:
      AnalyzingAlgorithmLostUClusters,
      AnalyzingAlgorithmLostVClusters,
      AnalyzingAlgorithmTotalUClusters,
      AnalyzingAlgorithmTotalVClusters,
      /// value type hit (with access to clusters via relations), stores vector< double >. defined in ..tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmClusterBased.h:
      AnalyzingAlgorithmLostUEDep,
      AnalyzingAlgorithmLostVEDep,
      AnalyzingAlgorithmTotalUEDep,
      AnalyzingAlgorithmTotalVEDep,
      NTypes // number of analyzingAlgorithmTypes available
    };


    /** checks if a type given is a valid type for an AlgoritmType */
    static bool isValidType(AlgoritmType::Type type)
    {
      return ((AlgoritmType::s_fromTypeToString.find(type) == AlgoritmType::s_fromTypeToString.end()) ? false : true);
    }


    /** checks if the name given is a valid name for an AlgoritmType */
    static bool isValidName(std::string type)
    {
      return ((AlgoritmType::s_fromStringToType.find(type) == AlgoritmType::s_fromStringToType.end()) ? false : true);
    }


    /** for given AlgoritmType the corresponding string-name will be returned. For invalid types, UnknownType will be passed */
    static std::string getTypeName(AlgoritmType::Type type)
    {
      auto pos = AlgoritmType::s_fromTypeToString.find(type);
      if (pos == AlgoritmType::s_fromTypeToString.end()) {
        B2ERROR("AlgoritmType::getTypeName(): given iD " << type << " is not a valid AlgoritmType, return AlgoritmType::UnknownType!");
        return AlgoritmType::s_fromTypeToString[UnknownType];
      }
      return pos->second;
    }


    /** for given string name of a AlgoritmType the corresponding AlgoritmType will be returned. For invalid types, UnknownType will be passed */
    static AlgoritmType::Type getTypeEnum(std::string type)
    {
      auto pos = AlgoritmType::s_fromStringToType.find(type);
      if (pos == AlgoritmType::s_fromStringToType.end()) {
        B2ERROR("AlgoritmType::getTypeName(): given iD " << type << " is not a valid AlgoritmType, return AlgoritmType::UnknownType!");
        return AlgoritmType::s_fromStringToType[std::string("UnknownType")];
      }
      return pos->second;
    }


    /** returns true if given AlgoritmType is a an algorithm which calculates a value in double for each TC passed, false if not */
    static bool isValueDoubleType(AlgoritmType::Type aType)
    { return aType < AlgoritmType::AnalyzingAlgorithmResidualPX and aType > AlgoritmType::AnalyzingAlgorithmBase; }

    /** returns true if given AlgoritmType is a an algorithm which calculates a residual in double for each TC passed, false if not */
    static bool isResidualDoubleType(AlgoritmType::Type aType)
    { return aType < AlgoritmType::AnalyzingAlgorithmLostUClusters and aType > AlgoritmType::AnalyzingAlgorithmValueQI; }

    /** returns true if given AlgoritmType is a an algorithm which calculates a value in int based on hits of each TC passed, false if not */
    static bool isHitValueIntType(AlgoritmType::Type aType)
    { return aType > AlgoritmType::AnalyzingAlgorithmResidualPositionXY and aType < AlgoritmType::AnalyzingAlgorithmLostUEDep; }

    /** returns true if given AlgoritmType is a an algorithm which calculates a value in vector< double> based on hits of each TC passed, false if not */
    static bool isHitValueVecDoubleType(AlgoritmType::Type aType)
    { return aType > AlgoritmType::AnalyzingAlgorithmTotalVClusters and aType < AlgoritmType::NTypes; }

  protected:
    /** static map allowing translation from a given type to its name stored as a string */
    static std::map<Type, std::string> s_fromTypeToString;

    /** static map allowing translation from a given name stored as a string to its type */
    static std::map<std::string, Type> s_fromStringToType;
  };



  /// equality-operators!
}
