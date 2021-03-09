/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoModule.h>
#include <top/dbobjects/TOPGeoBarSegment.h>
#include <top/dbobjects/TOPGeoPrism.h>
#include <vector>


namespace Belle2 {
  namespace TOP {

    /**
     * Base class with geometry data
     */
    class RaytracerBase {

    public:
      /**
       * Treatement of quartz geometry.
       */
      enum EGeometry {
        c_Unified = 0,   /**< single bar with average width and thickness */
        c_Segmented = 1  /**< segmented bars */
      };

      /**
       * Treatement of spherical mirror optics.
       */
      enum EOptics {
        c_SemiLinear = 0, /**< semi-linear approximation */
        c_Exact = 1       /**< exact optics */
      };

      /**
       * bar segment data in module local frame.
       */
      struct BarSegment {
        double A = 0;  /**< width (dimension in x) */
        double B = 0;  /**< thickness (dimension in y) */
        double zL = 0; /**< minimal z */
        double zR = 0; /**< maximal z */
        double reflectivity = 0; /**< surface reflectivity */

        /**
         * default constructor
         */
        BarSegment()
        {}

        /**
         * constructor from quartz segment.
         * @param bar quartz segment object from TOPGeometry
         * @param zLeft position of quartz segment left side (minimal z)
         */
        BarSegment(const TOPGeoBarSegment& bar, double zLeft);

        /**
         * constructor from joint quartz segments with average width and thickness.
         * @param module module object from TOPGeometry
         */
        explicit BarSegment(const TOPGeoModule& module);
      };

      /**
       * spherical mirror data in module local frame.
       */
      struct Mirror {
        double xc = 0; /**< center of curvature in x */
        double yc = 0; /**< center of curvature in y */
        double zc = 0; /**< center of curvature in z */
        double R = 0;  /**< radius */
        double zb = 0; /**< minimum of mirror surface in z */
        double reflectivity = 0; /**< mirror reflectivity */

        /**
         * default constructor
         */
        Mirror()
        {}

        /**
         * constructor from TOP module.
         * @param module module object from TOPGeometry
         */
        explicit Mirror(const TOPGeoModule& module);
      };

      /**
       * prism data in module local frame.
       */
      struct Prism {
        double A = 0;     /**< width (dimension in x) */
        double B = 0;     /**< thickness at bar (dimension in y) */
        double yUp = 0;   /**< maximal y of exit window */
        double yDown = 0; /**< minimal y of exit window */
        double zL = 0;    /**< minimal z */
        double zR = 0;    /**< maximal z, i.e position of prism-bar joint */
        double zFlat = 0; /**< z where flat continues to slanted surface */
        double zD = 0;    /**< detector (photo-cathode) position */
        double slope = 0; /**< slope of slanted surface (dy/dz) */
        int k0 = 0; /**< index of true prism in the vector 'unfoldedWindows' */
        std::vector<TOPGeoPrism::UnfoldedWindow> unfoldedWindows; /**< unfolded prism exit windows */

        /**
         * default constructor
         */
        Prism()
        {}

        /**
         * constructor from TOP module.
         * @param module module object from TOPGeometry
         */
        explicit Prism(const TOPGeoModule& module);
      };


      /**
       * Constructor
       * @param moduleID slot ID
       * @param geometry treatement of quartz geometry
       * @param optics treatement of spherical mirror optics
       */
      RaytracerBase(int moduleID, EGeometry geometry = c_Unified, EOptics optics = c_SemiLinear);

      /**
       * Returns slot ID.
       * @return slot ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns quartz geometry treatement.
       * @return quartz geometry treatement
       */
      EGeometry getGeometry() const {return m_geometry;}

      /**
       * Returns treatement of spherical mirror optics.
       * @return spherical mirror optics
       */
      EOptics getOptics() const {return m_optics;}

      /**
       * Returns geometry data of bar segments.
       * @return geometry data of bar segments
        */
      const std::vector<BarSegment>& getBars() const {return m_bars;}

      /**
       * Returns geometry data of spherical mirror.
       * @return geometry data of spherical mirror
       */
      const Mirror& getMirror() const {return m_mirror;}

      /**
       * Returns geometry data of prism.
       * @return geometry data of prism
       */
      const Prism& getPrism() const {return m_prism;}


    protected:

      int m_moduleID = 0; /**< slot ID */
      EGeometry m_geometry = c_Unified; /**< quartz geometry */
      EOptics m_optics = c_SemiLinear;  /**< spherical mirror optics */

      std::vector<BarSegment> m_bars; /**< geometry data of bar segments */
      Mirror m_mirror; /**< spherical mirror geometry data */
      Prism m_prism; /**< prism geometry data */

    };

  } // namespace TOP
} // namespace Belle2

