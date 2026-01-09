/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackingUtilities/geometry/Vector2D.h>

#include <sstream>
#include <ostream>

using namespace Belle2;
using namespace TrackingUtilities;

Vector2D::Vector2D(const ROOT::Math::XYVector& xyVector)
  : m_x(xyVector.X()), m_y(xyVector.Y())
{
}

Vector2D& Vector2D::operator=(const ROOT::Math::XYVector& xyVector)
{
  m_x = xyVector.X();
  m_y = xyVector.Y();
  return *this;
}

Vector2D::operator const ROOT::Math::XYVector() const
{
  return ROOT::Math::XYVector(x(), y());
}

std::ostream& TrackingUtilities::operator<<(std::ostream& output, const Vector2D& vector2D)
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
