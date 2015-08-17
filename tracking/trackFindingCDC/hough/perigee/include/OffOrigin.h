/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/numerics/Sign.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Takes a basic object and passivelly moves it to any origin
     *  All subordinary coordinates have to be interpreted relative to this origin.
     */
    template<class T>
    class OffOrigin : public T {

    public:
      // Type of the base class
      using Super = T;

      /// Using the constructor of the base class.
      using T::T;

    public:
      /// The houghBox to which this object correspondes.
      typedef typename T::HoughBox HoughBox;

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      inline SignType getDistanceSign(const HoughBox* houghBox,
                                      const Vector2D& pos2D,
                                      const double signedDriftLength,
                                      const Vector2D& movePerZ = Vector2D(0.0, 0.0)) const
      {
        Vector2D localPos2D = pos2D - m_localOrigin;
        return Super::getDistanceSign(houghBox, localPos2D, signedDriftLength, movePerZ);
      }

    public:
      /// Getter for the local origin relative to which the parameters of the hough space are understood
      const Vector2D& getLocalOrigin() const
      { return m_localOrigin; }

      /// Setter for the local origin relative to which the parameters of the hough space are understood
      void setLocalOrigin(const Vector2D& localOrigin)
      { m_localOrigin = localOrigin; }

    private:
      /// The local origin to which the original plane is displaced.
      Vector2D m_localOrigin = Vector2D(0.0, 0.0);
    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
