/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <sstream>
#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

std::ostream& TrackFindingCDC::operator<<(std::ostream& output, const Vector2D& vector2D)
{
  output << "Vector2D(" << vector2D.x() << "," << vector2D.y() << ")";
  return output;
}

std::string Vector2D::__str__() const
{
  std::stringstream sstream;
  sstream << *this;
  return sstream.str();
}
