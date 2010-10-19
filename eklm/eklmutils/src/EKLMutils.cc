/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/





#include <eklm/eklmutils/EKLMutils.h>

#include <iostream>
#include <string>


namespace Belle2 {


  // get plane

  std::string EKLMNameManipulator::getPlane(std::string *stripName)
  {
    std::string str;
    try {
      str = stripName->substr(stripName->find("Plane") + 6, 1); // get from "r" to the end
    } catch (exception& e) {
      ERROR("exception caught:" << e.what() << " Strip name does not point plane!");
    }
    return str;
  }

  std::string EKLMNameManipulator::getPlane(const char * stripName)
  {
    return getPlane(std::string(stripName));
  }

  std::string EKLMNameManipulator::getPlane(TGeoVolume * strip)
  {
    return getPlane(strip->GetName());
  }

  std::string EKLMNameManipulator::getPlane(std::string stripName)
  {
    std::string str;
    try {
      str = stripName.substr(stripName.find("Plane") + 6, 1); // get from "Sector" to the end
    } catch (exception& e) {
      ERROR("exception caught:" << e.what() << " Strip name does not point sector!");
    }
    return str;
  }



  // get sector name


  std::string EKLMNameManipulator::getSectorName(std::string *stripName)
  {
    std::string str;
    try {
      str = stripName->substr(stripName->find("Sector")); // get from "Sector" to the end
    } catch (exception& e) {
      ERROR("exception caught:" << e.what() << " Strip name does not point sector!");
    }
    return str;
  }

  std::string EKLMNameManipulator::getSectorName(const char * stripName)
  {
    return getSectorName(std::string(stripName));
  }
  std::string EKLMNameManipulator::getSectorName(std::string stripName)
  {
    std::string str;
    try {
      str = stripName.substr(stripName.find("Sector")); // get from "Sector" to the end
    } catch (exception& e) {
      ERROR("exception caught:" << e.what() << " Strip name does not point sector!");
    }
    return str;
  }
  std::string EKLMNameManipulator::getSectorName(TGeoVolume * strip)
  {
    return getSectorName(strip->GetName());
  }


}

