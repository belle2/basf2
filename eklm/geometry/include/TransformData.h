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
#include <eklm/geometry/GeometryData.h>

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
       * Source of displacement (alignment) data.
       */
      enum Displacement {
        c_None,         /**< Displacement is not used. */
        c_Displacement, /**< Use displacement data (for geometry). */
        c_Alignment,    /**< Use alignment data (for everything else). */
      };

      /**
       * Constructor.
       * @param[in] global           If true, load global transformations
       *                             (false - local).
       * @param[in] displacementType Displacement type.
       */
      TransformData(bool global, Displacement displacementType);

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
       * Get additional displacement for plane internal volumes.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       */
      const HepGeom::Transform3D*
      getPlaneDisplacement(int endcap, int layer, int sector, int plane) const;

      /**
       * Get segment transformation.
       * @param[in] endcap  Endcap number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] segment Segment number.
       */
      const HepGeom::Transform3D*
      getSegmentTransform(int endcap, int layer, int sector, int plane,
                          int segment) const;

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
       * Get strip global to local transformation by hit.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @param[in] strip  Strip number.
       * @return Transformation.
       */
      const HepGeom::Transform3D*
      getStripGlobalToLocal(int endcap, int layer, int sector, int plane,
                            int strip) const;

      /**
       * Check if strips intersect, and find intersection point if yes.
       * @param[in]  hit1     First hit.
       * @param[in]  hit2     Second hit.
       * @param[out] cross    Crossing point (coordinate unit is cm).
       * @param[out] d1       Distance from hit to SiPM of strip 1, cm.
       * @param[out] d2       Distance from hit to SiPM of strip 2, cm.
       * @param[out] sd       Shortest distance between strips, cm.
       *                      Or if second strip is closer to interaction point,
       *                      then (- shortest distance).
       * @param[in]  segments Check if segments intersect (may need to turn this
       *                      check off for debugging).
       * @return True if strips intersect.
       */
      bool intersection(EKLMDigit* hit1, EKLMDigit* hit2,
                        HepGeom::Point3D<double>* cross,
                        double* d1, double* d2, double* sd,
                        bool segments = true);

    private:

      /**
       * Make transformations global from local.
       * @param[in,out] dat Transformation data.
       */
      void transformsToGlobal();

      /** Geometry data. */
      const GeometryData* m_GeoDat;

      /** Endcap transformations. */
      HepGeom::Transform3D* m_Endcap;

      /** Layer transformations. */
      HepGeom::Transform3D** m_Layer;

      /** Sector transformations. */
      HepGeom::Transform3D*** m_Sector;

      /** Plane transformations. */
      HepGeom::Transform3D**** m_Plane;

      /** Plane internal volumes displacements. */
      HepGeom::Transform3D**** m_PlaneDisplacement;

      /** Segment transformations. */
      HepGeom::Transform3D***** m_Segment;

      /** Strip transformations. */
      HepGeom::Transform3D***** m_Strip;

      /** Inverse strip transformations. */
      HepGeom::Transform3D***** m_StripInverse;

    };

  }

}

#endif

