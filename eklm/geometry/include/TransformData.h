/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMTRANSFORMDATA_H
#define EKLMTRANSFORMDATA_H

/* External headers. */
#include <CLHEP/Geometry/Transform3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>

/**
 * @file
 * EKLM transformation and alignment data operations.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * Transformation data.
     * All data and function results are in CLHEP units unless noted otherwise.
     */
    class TransformData {

    public:

      /**
       * Constructor.
       * @param[in] global            If true, load global transformations
       *                              (false - local).
       * @param[in] alignmentDataFile Alignment data file (NULL - use default
       *                              positions).
       */
      TransformData(bool global, const char* alignmentDataFile);

      /**
       * Destructor.
       */
      ~TransformData();

      /**
       * Get endcap transformation.
       * @param[in] endcap Endcap number.
       */
      const HepGeom::Transform3D* getEndcapTransform(int endcap) const;

      /**
       * Get layer transformation.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       */
      const HepGeom::Transform3D*
      getLayerTransform(int endcap, int layer) const;

      /**
       * Get sector transformation.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       */
      const HepGeom::Transform3D*
      getSectorTransform(int endcap, int layer, int sector) const;

      /**
       * Get plane transformation.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       */
      const HepGeom::Transform3D*
      getPlaneTransform(int endcap, int layer, int sector, int plane) const;

      /**
       * Get strip local to global transformation by hit.
       * @param[in] hit Hit.
       * @return Transformation.
       */
      const HepGeom::Transform3D* getStripLocalToGlobal(EKLMDigit* hit) const;

      /**
       * Get strip global to local transformation by hit.
       * @param[in] hit Hit.
       * @return Transformation.
       */
      const HepGeom::Transform3D* getStripGlobalToLocal(EKLMDigit* hit) const;

      /**
       * Get strip transformation.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @param[in] strip  Strip number.
       */
      const HepGeom::Transform3D*
      getStripTransform(int endcap, int layer, int sector, int plane,
                        int strip) const;

      /**
       * Check if strips intersect, and find intersection point if yes.
       * @param[in] hit1   First hit.
       * @param[in] hit2   Second hit.
       * @param[out] cross Crossing point (coordinate unit is cm).
       * @param[out] d1    Distance from hit to SiPM of strip 1, cm.
       * @param[out] d2    Distance from hit to SiPM of strip 2, cm.
       * @param[out] sd    Shortest distance between strips, cm.
       *                   Or if second strip is closer to interaction point,
       *                   then (- shortest distance).
       * @return True if strips intersect.
       */
      bool intersection(EKLMDigit* hit1, EKLMDigit* hit2,
                        HepGeom::Point3D<double>* cross,
                        double* d1, double* d2, double* sd);

    private:

      /**
       * Make transformations global from local.
       * @param[in,out] dat Transformation data.
       */
      void transformsToGlobal();

      /** Endcap transformations. */
      HepGeom::Transform3D m_Endcap[2];

      /** Layer transformations. */
      HepGeom::Transform3D m_Layer[2][14];

      /** Sector transformations. */
      HepGeom::Transform3D m_Sector[2][14][4];

      /** Plane transformations. */
      HepGeom::Transform3D m_Plane[2][14][4][2];

      /** Strip transformations. */
      HepGeom::Transform3D m_Strip[2][14][4][2][75];

      /** Inverse strip transformations. */
      HepGeom::Transform3D m_StripInverse[2][14][4][2][75];

    };

  }

}

#endif

