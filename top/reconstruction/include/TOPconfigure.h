/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPCONFIGURE_H
#define TOPCONFIGURE_H

#include <top/geometry/TOPGeometryPar.h>

extern "C" {void top_geo_dump_();}

namespace Belle2 {
  namespace TOP {

    /**
     * Configure TOP geometry for reconstruction: provides interface to fortran code
     */
    class TOPconfigure {
    public:
      /**
       * constructor (which does the stuff if not configured yet)
       */
      TOPconfigure();

      /**
       * print configuration to std output (using fortran print)
       */
      void print() const {top_geo_dump_();}

      /**
       * Returns configuration status
       * @return status
       */
      bool isConfigured() const {return m_configured;}

      /**
       * Returns inner radius of space for TOP bars
       * @return inner radius
       */
      double getR1() const {return m_R1;}

      /**
       * Returns outer radius of space for TOP bars
       * @return outer radius
       */
      double getR2() const {return m_R2;}

      /**
       * Returns minimal z of space for TOP bars
       * @return minimal z
       */
      double getZ1() const {return m_Z1;}

      /**
       * Returns maximal z of space for TOP bars
       * @return maximal z
       */
      double getZ2() const {return m_Z2;}

      /**
       * Returns time range of TDC
       * @return time range
       */
      double getTDCTimeRange() const {return m_timeRange;}

    private:
      static bool m_configured; /**< set to true when done */
      TOPGeometryPar* m_topgp;   /**< geometry parameters */
      double m_R1;   /**< inner radius */
      double m_R2;   /**< outer radius */
      double m_Z1;   /**< backward z */
      double m_Z2;   /**< forward z */
      double m_timeRange; /**< time range of TDC */

    };

  } // top namespace
} // Belle2 namespace

#endif
