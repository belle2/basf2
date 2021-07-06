/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/dataobjects/KLMDigit.h>

/* CLHEP headers. */
#include <CLHEP/Geometry/Transform3D.h>

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
       * Copy constructor (disabled).
       */
      TransformData(const TransformData&) = delete;

      /**
       * Operator = (disabled).
       */
      TransformData& operator=(const TransformData&) = delete;

      /**
       * Destructor.
       */
      ~TransformData();

      /**
       * Get section transformation.
       * @param[in] section Section number.
       */
      const HepGeom::Transform3D* getSectionTransform(int section) const;

      /**
       * Get layer transformation.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       */
      const HepGeom::Transform3D*
      getLayerTransform(int section, int layer) const;

      /**
       * Get sector transformation.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       */
      const HepGeom::Transform3D*
      getSectorTransform(int section, int layer, int sector) const;

      /**
       * Get plane transformation.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       */
      const HepGeom::Transform3D*
      getPlaneTransform(int section, int layer, int sector, int plane) const;

      /**
       * Get additional displacement for plane internal volumes.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       */
      const HepGeom::Transform3D*
      getPlaneDisplacement(int section, int layer, int sector, int plane) const;

      /**
       * Get segment transformation.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] segment Segment number.
       */
      const HepGeom::Transform3D*
      getSegmentTransform(int section, int layer, int sector, int plane,
                          int segment) const;

      /**
       * Get strip local to global transformation by hit.
       * @param[in] hit Hit.
       * @return Transformation.
       */
      const HepGeom::Transform3D* getStripLocalToGlobal(KLMDigit* hit) const;

      /**
       * Get strip global to local transformation by hit.
       * @param[in] hit Hit.
       * @return Transformation.
       */
      const HepGeom::Transform3D* getStripGlobalToLocal(KLMDigit* hit) const;

      /**
       * Get strip transformation.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] strip   Strip number.
       */
      const HepGeom::Transform3D*
      getStripTransform(int section, int layer, int sector, int plane,
                        int strip) const;

      /**
       * Get strip global to local transformation by hit.
       * @param[in] section Section number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] strip   Strip number.
       * @return Transformation.
       */
      const HepGeom::Transform3D*
      getStripGlobalToLocal(int section, int layer, int sector, int plane,
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
      bool intersection(KLMDigit* hit1, KLMDigit* hit2,
                        HepGeom::Point3D<double>* cross,
                        double* d1, double* d2, double* sd,
                        bool segments = true) const;

      /**
       * Get sector by position.
       * @param[in] section  Section number.
       * @param[in] position Position.
       */
      int getSectorByPosition(int section,
                              const HepGeom::Point3D<double>& position) const;


      /**
       * Find strips by intersection.
       * @param[in]  intersection Intersection point.
       * @param[out] strip1       Strip 1 global number.
       * @param[out] strip2       Strip 2 global number.
       * @return 0 on success, -1 on error.
       */
      int getStripsByIntersection(const HepGeom::Point3D<double>& intersection,
                                  int* strip1, int* strip2) const;

    private:

      /**
       * Make transformations global from local.
       * @param[in,out] dat Transformation data.
       */
      void transformsToGlobal();

      /** Element numbers. */
      const EKLMElementNumbers* m_ElementNumbers;

      /** Geometry data. */
      const GeometryData* m_GeoDat;

      /** Section transformations. */
      HepGeom::Transform3D* m_Section;

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
