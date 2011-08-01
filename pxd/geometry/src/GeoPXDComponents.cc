/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Christian Oswald,           *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geometry/GeoPXDCreator.h>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace Belle2 {
  namespace pxd {
    GeoPXDPlacement::GeoPXDPlacement(const std::string &name, double u, double v, std::string w): name(name), u(u), v(v)
    {
      boost::to_lower(w);
      if (w == "below")        this->w = c_below;
      else if (w == "bottom")  this->w = c_bottom;
      else if (w == "center")  this->w = c_center;
      else if (w == "top")     this->w = c_top;
      else if (w == "above")   this->w = c_above;
      else  B2FATAL("Unknown z-placement for PXD Component: " << w << ", check xml file");
    }
  }
}
