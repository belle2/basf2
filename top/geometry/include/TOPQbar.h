/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPQBAR_H
#define TOPQBAR_H

#include <TVector3.h>

namespace Belle2 {
  namespace TOP {

    /**
     * quartz bar geometry
     */
    class TOPQbar {

    public:

      /**
       * constructor
       * @param width Q-bar width
       * @param thickness Q-bar thickness
       * @param z1 z-coordinate of the backward Q-bar end
       * @param z2 z-coordinate of the forward Q-bar end
       * @param R radius of inner Q-bar surface
       * @param dx shift in x
       * @param phi Q-bar azimuthal angle
       * @param barID Q-bar ID
       */
      TOPQbar(double width, double thickness, double z1, double z2, double R, double dx,
              double phi, int barID);

      /**
       * destructor
       */
      ~TOPQbar();

      /**
       * Returns Q-bar ID
       * @return ID
       */
      int getBarID() const {return m_barID;}

      /**
       * Returns Q-bar width
       * @return width
       */
      double getWidth() const {return m_width;}

      /**
       * Returns Q-bar thickness
       * @return thickness
       */
      double getThickness() const {return m_thickness;}

      /**
       * Returns Q-bar length
       * @return length
       */
      double getLength() const {return m_length;}

      /**
       * Returns z-coordinate of the backward Q-bar end
       * @return backward z-coordinate
       */
      double getBackwardZ() const {return m_backwardZ;}

      /**
       * Returns z-coordinate of the forward Q-bar end
       * @return forward z-coordinate
       */
      double getForwardZ() const {return m_backwardZ + m_length;}

      /**
       * Returns radius of Q-bar center (inner radius + thickness/2)
       * @return radius
       */
      double getRadius() const {return m_radius;}

      /**
       * Returns Q-bar shift along local x-coordinate
       * @return shift in x
       */
      double getShiftInX() const {return m_shiftInX;}

      /**
       * Returns Q-bar shift along z-coordinate
       * @return shift in z
       */
      double getShiftInZ() const {return m_shiftInZ;}

      /**
       * Returns Q-bar azimuthal angle
       * @return azimuthal angle
       */
      double getPhi() const {return m_phi;}

      /**
       * Returns cosine of Q-bar azimuthal angle
       * @return cos(phi)
       */
      double getCosPhi() const {return m_cosPhi;}

      /**
       * Returns sine of Q-bar azimuthal angle
       * @return sin(phi)
       */
      double getSinPhi() const {return m_sinPhi;}

      /**
       * Transforms 3D point from Belle II to Q-bar frame
       * @return 3D point in local frame
       */
      TVector3 pointToLocal(const TVector3& point) const;

      /**
       * Transforms momentum vector from Belle II to Q-bar frame
       * @return momentum vector in local frame
       */
      TVector3 momentumToLocal(const TVector3& momentum) const;

      /**
       * Transforms 3D point from Q-bar frame to Belle II frame
       * @return 3D point in global frame
       */
      TVector3 pointToGlobal(const TVector3& point) const;

      /**
       * Transforms momentum vector from Q-bar frame to Belle II frame
       * @return momentum vector in global frame
       */
      TVector3 momentumToGlobal(const TVector3& momentum) const;


    private:

      int m_barID;        /**< Q-bar ID */
      double m_width;     /**< Q-bar width */
      double m_thickness; /**< Q-bar thickness */
      double m_length;    /**< Q-bar length */
      double m_backwardZ; /**< z-coordinate of the backward Q-bar end */
      double m_radius;    /**< radius of Q-bar center (inner radius + thickness/2) */
      double m_shiftInX;  /**< Q-bar shift in x */
      double m_shiftInZ;  /**< Q-bar shift in z */
      double m_phi;       /**< Q-bar azimuthal angle */
      double m_cosPhi;    /**< cos(m_phi) */
      double m_sinPhi;    /**< sin(m_phi) */

    };

  } // TOP namespace
} // Belle2 namespace

#endif
