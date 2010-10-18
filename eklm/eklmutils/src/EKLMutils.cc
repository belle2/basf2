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




namespace Belle2 {


  std::string EKLMNameManipulator::getSectorName(std::string *stripName)
  {
    return stripName->substr(stripName->find("Sector")); // get from "Sector" to the end
  }
  std::string EKLMNameManipulator::getSectorName(TGeoVolume * strip)
  {
    std::string name = strip->GetName();
    return getSectorName(&name);
  }


}

