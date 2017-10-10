/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <TVector2.h>

#include <sstream>
#include <ostream>

using namespace Belle2;
using namespace TrackFindingCDC;

Vector2D::Vector2D(const TVector2& tVector2)
  : m_x(tVector2.X())
  , m_y(tVector2.Y())
{
}

Vector2D& Vector2D::operator=(const TVector2& tVector2)
{
  setX(tVector2.X());
  setY(tVector2.Y());
  return *this;
}

Vector2D::operator const TVector2()
{
  return TVector2(x(), y());
}

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
