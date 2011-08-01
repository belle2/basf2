/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOMETRY_UTILITIES_H
#define GEOMETRY_UTILITIES_H

#include <string>
//class G4Colour;
class G4LogicalVolume;

namespace Belle2 {
  namespace geometry {
//    G4Colour parseColor(std::string colorString);
    void setColor(G4LogicalVolume &volume, const std::string &color);
    void setVisibility(G4LogicalVolume &volume, bool visible);
  }
} //Belle2 namespace
#endif
