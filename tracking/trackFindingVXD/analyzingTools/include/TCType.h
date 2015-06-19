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
  /** Small class containing enums and converter from and to strings */
  class TCType {
  public:

    /** allows classifying TCs */
    enum Type {
      UnknownType, // the type of this TC is not even unclassified but completely unknown, normally a hint for errors
      Unclassified, // a TC which was not classified yet
      Lost, // reference TCs which were not found by test TF
      RefOutlier, // reference TC which was rejected by some cuts or thresholds set by some classifier
      Ghost, // did not reach m_PARAMqiThreshold
      SmallStump, // TC too short
      Clone, // reached threshold but for the same reference TC a better partner was already found
      Contaminated, // more than m_PARAMqiThreshold in purity
      Clean, // test TC has 100% purity for the particle type but not all hits of reference TC were found
      Perfect, // test TC is identical with a reference TC (have exactly the same hits and no extra ones)
      AllTCTypes, // all other test types are smaller than this value
      Reference, // reference TC
      NTypes // number of tcTypes available
    };


    /** for given TCType the corresponding string-name will be returned. For invalid types, UnknownType will be passed */
    static std::string getTypeName(TCType::Type type)
    {
      auto pos = TCType::m_fromTypeToString.find(type);
      if (pos == TCType::m_fromTypeToString.end()) {
        B2ERROR("TCType::getTypeName(): given iD " << type << " is not a valid TCType, returnint TCType::UnknownType!")
        return TCType::m_fromTypeToString[UnknownType];
      }
      return pos->second;
    }


    /** for given string name of a TCType the corresponding TCType will be returned. For invalid types, UnknownType will be passed */
    static TCType::Type getTypeEnum(std::string type)
    {
      auto pos = TCType::m_fromStringToType.find(type);
      if (pos == TCType::m_fromStringToType.end()) {
        B2ERROR("TCType::getTypeName(): given iD " << type << " is not a valid TCType, returnint TCType::UnknownType!")
        return TCType::m_fromStringToType[std::string("UnknownType")];
      }
      return pos->second;
    }


    /** returns true if given TCType is a reference-Type, false if not */
    static bool isReference(TCType::Type aType)
    {
      return aType == TCType::Reference
             or aType == TCType::Lost
             or aType == TCType::RefOutlier;
    }


    /** returns true if given TCType is a testTC-Type, false if not */
    static bool isTestTC(TCType::Type aType)
    {
      return aType > TCType::RefOutlier
             and aType < TCType::AllTCTypes;
    }


    /** returns true if given TCType is a testTC-Type and did successfully reconstruct a track */
    static bool isGoodTestTC(TCType::Type aType)
    {
      return aType > TCType::Clone
             and aType < TCType::AllTCTypes;
    }


  protected:
    /** static map allowing translation from a given type to its name stored as a string */
    static std::map<Type, std::string> m_fromTypeToString;

    /** static map allowing translation from a given name stored as a string to its type */
    static std::map<std::string, Type> m_fromStringToType;
  };



  /// equality-operators!
}
