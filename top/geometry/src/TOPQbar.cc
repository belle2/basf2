/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/TOPQbar.h>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    TOPQbar::TOPQbar(double width, double thickness, double z1, double z2,
                     double R, double dx, double phi, int barID)
    {
      m_barID = barID;
      m_width = width;
      m_thickness = thickness;
      m_length = z2 - z1;
      m_backwardZ = z1;
      m_radius = R + thickness / 2.0;
      m_shiftInX = dx;
      m_shiftInZ = (z1 + z2) / 2.0;
      m_phi = phi;
      m_cosPhi = cos(phi);
      m_sinPhi = sin(phi);
    }


    TOPQbar::~TOPQbar()
    {
    }

    TVector3 TOPQbar::pointToLocal(const TVector3& point) const
    {
      double x = point.X() * m_cosPhi + point.Y() * m_sinPhi;
      double y = point.Y() * m_cosPhi - point.X() * m_sinPhi;
      double z = point.Z();
      return TVector3(x - m_shiftInX,
                      y - m_radius,
                      z - m_shiftInZ);
    }

    TVector3 TOPQbar::momentumToLocal(const TVector3& momentum) const
    {
      double x = momentum.X() * m_cosPhi + momentum.Y() * m_sinPhi;
      double y = momentum.Y() * m_cosPhi - momentum.X() * m_sinPhi;
      double z = momentum.Z();
      return TVector3(x, y, z);

    }

    TVector3 TOPQbar::pointToGlobal(const TVector3& point) const
    {
      double x = point.X() + m_shiftInX;
      double y = point.Y() + m_radius;
      double z = point.Z() + m_shiftInZ;
      return TVector3(x * m_cosPhi - y * m_sinPhi,
                      y * m_cosPhi + x * m_sinPhi,
                      z);
    }

    TVector3 TOPQbar::momentumToGlobal(const TVector3& momentum) const
    {
      double x = momentum.X();
      double y = momentum.Y();
      double z = momentum.Z();
      return TVector3(x * m_cosPhi - y * m_sinPhi,
                      y * m_cosPhi + x * m_sinPhi,
                      z);
    }

  } // TOP namespace
} // Belle2 namespace
