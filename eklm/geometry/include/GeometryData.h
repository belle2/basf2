/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_GEOMETRYDATA_H
#define EKLM_GEOMETRYDATA_H

/* External headers. */
#include <CLHEP/Geometry/Transform3D.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMElementNumbers.h>
#include <eklm/dbobjects/EKLMGeometry.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/gearbox/GearDir.h>

/**
 * @file
 * EKLM geometry data.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * EKLM geometry data.
     * All data and function results are in CLHEP units unless noted otherwise.
     */
    class GeometryData : public EKLMGeometry {

    public:

      /** Geometry data source. */
      enum DataSource {

        /** Gearbox (XML). */
        c_Gearbox,

        /** Database. */
        c_Database,

      };

      /**
       * Instantiation.
       * @param[in] dataSource Data source.
       */
      static const GeometryData& Instance(enum DataSource dataSource =
                                            c_Gearbox);

      /**
       * Save geometry data to database.
       */
      void saveToDatabase(const IntervalOfValidity& iov) const;

      /**
       * Get strip length.
       * @param[in] strip Strip number.
       */
      double getStripLength(int strip) const;

      /**
       * Get number of strips with different lengths.
       */
      int getNStripsDifferentLength() const;

      /**
       * Get index in length-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripLengthIndex(int positionIndex) const;

      /**
       * Get index in position-based array.
       * @param[in] positionIndex index in position-based array.
       */
      int getStripPositionIndex(int lengthIndex) const;

      /**
       * Check if z coordinate may be in EKLM.
       * @param[in] z Z coordinate (cm).
       *
       * Warning: this is not a complete check!
       */
      bool hitInEKLM(double z) const;

      /**
       * Get endcap transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of endcap.
       * @details
       * Numbers start from 0.
       */
      void getEndcapTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get layer transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of layer.
       * @details
       * Numbers start from 0.
       */
      void getLayerTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get sector transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of sector.
       * @details
       * Numbers start from 0.
       */
      void getSectorTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get plane transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of plane.
       * @details
       * Numbers start from 0.
       */
      void getPlaneTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get strip transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of strip.
       * @details
       * Numbers start from 0.
       */
      void getStripTransform(HepGeom::Transform3D* t, int n) const;

      /**
       * Get plastic sheet element transformation.
       * @param[out] t Transformation.
       * @param[in]  n Number of list.
       * @details
       * Numbers start from 0.
       * This function is intended for construction of the list solids, so
       * the transformation does not include z shift.
       */
      void getSheetTransform(HepGeom::Transform3D* t, int n) const;

    private:

      /**
       * Constructor.
       * @param[in] dataSource Data source.
       */
      GeometryData(enum DataSource dataSource);

      /**
       * Destructor.
       */
      ~GeometryData();

      /**
       * Read endcap structure geometry data.
       * @param[in] gd Current GearDir.
       */
      void readEndcapStructureGeometry(const GearDir& gd);

      /**
       * Initialize from Gearbox (XML).
       */
      void initializeFromGearbox();

      /**
       * Initialize from database.
       */
      void initializeFromDatabase();

      /**
       * Calculate sector support geometry data.
       */
      void calculateSectorSupportGeometry();

      /**
       * Fill strip index arrays.
       */
      void fillStripIndexArrays();

      /**
       * Read strip parameters from XML database.
       */
      void readXMLDataStrips();

      /**
       * Calculate shield geometry data.
       */
      void calculateShieldGeometry();

      /** Copy of data in this class used to write it to database. */
      EKLMGeometry* m_Geometry;

      /** Number of strips with different lengths in one plane. */
      int m_nStripDifferent;

      /** Number of strip in length-based array. */
      int* m_StripAllToLen;

      /** Number of strip in position-based array. */
      int* m_StripLenToAll;

      /** Minimal z coordinate of the forward endcap. */
      double m_MinZForward;

      /** Maximal z coordinate of the backward endcap. */
      double m_MaxZBackward;

    };

  }

}

#endif

