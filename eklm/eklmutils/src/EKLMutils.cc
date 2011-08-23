/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/geoeklm/GeoEKLMBelleII.h>

#include <eklm/eklmutils/EKLMutils.h>
#include <iostream>
#include <string>
#include "boost/lexical_cast.hpp"

using namespace std;
namespace Belle2 {


  std::string EKLMNameManipulator::getVolumeName(const char * stripName, const char * elementName)
  {
    return getVolumeName(std::string(stripName), std::string(elementName));
  }

  std::string EKLMNameManipulator::getVolumeName(std::string stripName, std::string elementName)
  {
    std::string str;
    try {
      str = stripName.substr(stripName.find(elementName)); // get from elementName to the end
    } catch (exception& e) {
      B2ERROR("exception caught:" << e.what() << " Strip name does not point " + elementName + "!");
    }
    //    std::cout << "EKLMNameManipulator::getName(" << stripName << "," << elementName << ")=" << str << std::endl;
    return str;
  }


  int EKLMNameManipulator::getVolumeNumber(const char * stripName, const char * elementName)
  {
    return getVolumeNumber(std::string(stripName), std::string(elementName));
  }


  int EKLMNameManipulator::getVolumeNumber(std::string stripName, std::string elementName)
  {
    std::string str;
    try {
      size_t pos1 = stripName.find(elementName + "_") + 1 + elementName.size();
      size_t pos2 = stripName.find("_", pos1 + 1);
      str = stripName.substr(pos1, pos2 - pos1); // get string btw elementName_  and next _
    } catch (exception& e) {
      B2ERROR("exception caught:" << e.what() << elementName + " name does not point a number!");
    }
    return boost::lexical_cast<int>(str);
  }



  std::string  EKLMNameManipulator::getNodePath(const char * stripName)
  {
    return getNodePath(std::string(stripName));
  }

  std::string EKLMNameManipulator::getNodePath(std::string stripName)
  {
    std::string path = \
                       std::string("/Top_1/EKLM_1") +          \
                       std::string("/") + getVolumeName(stripName, "Endcap") +   \
                       std::string("_") + boost::lexical_cast<std::string>(getVolumeNumber(stripName, "Endcap")) + \
                       std::string("/") + getVolumeName(stripName, "Layer") +    \
                       std::string("_") + boost::lexical_cast<std::string>(getVolumeNumber(stripName, "Layer")) + \
                       std::string("/") + getVolumeName(stripName, "Sector") +   \
                       std::string("_") + boost::lexical_cast<std::string>(getVolumeNumber(stripName, "Sector")) + \
                       std::string("/") + getVolumeName(stripName, "Plane") +    \
                       std::string("_") + boost::lexical_cast<std::string>(getVolumeNumber(stripName, "Plane")) + \
                       std::string("/") + getVolumeName(stripName, "Strip") +    \
                       std::string("_") + boost::lexical_cast<std::string>(getVolumeNumber(stripName, "Strip"));
    // tempopary. Used for the procedure checks
//    if (!gGeoManager->CheckPath(path.c_str()))
//      B2ERROR("Something goes wrong! Strip path is not found!");
    return path;
  }
}
