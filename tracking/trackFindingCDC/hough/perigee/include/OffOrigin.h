/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/topology/ILayer.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Takes a basic object and passivelly moves it to any origin
     *  All subordinary coordinates have to be interpreted relative to this origin.
     */
    template<class T>
    class OffOrigin : public T {

    public:
      /// Type of the base class
      using Super = T;

      /// Using the constructor of the base class.
      using T::T;

    public:
      /// The houghBox to which this object correspondes.
      using HoughBox = typename T::HoughBox;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      ESign getDistanceSign(const HoughBox& houghBox,
                            float x,
                            float y,
                            float signedDriftLength,
                            float dxdz = 0,
                            float dydz = 0,
                            ILayer /*iCLayer*/ = -1) const
      {
        return Super::getDistanceSign(houghBox,
                                      x - m_localOriginX,
                                      y - m_localOriginY,
                                      signedDriftLength,
                                      dxdz,
                                      dydz,
                                      -1);
      }

    public:
      /// Getter for the local origin relative to which the parameters of the hough space are understood
      Vector2D getLocalOrigin() const
      { return Vector2D(m_localOriginX, m_localOriginY); }

      /// Setter for the local origin relative to which the parameters of the hough space are understood
      void setLocalOrigin(const Vector2D& localOrigin)
      {
        m_localOriginX = localOrigin.x();
        m_localOriginY = localOrigin.y();
      }
    private:
      /// The local origin x to which the original plane is displaced.
      float m_localOriginX = 0.0;

      /// The local origin y to which the original plane is displaced.
      float m_localOriginY = 0.0;

    };
  }
}
