/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_SENSORINFO_H
#define SVD_SENSORINFO_H

#include <vxd/geometry/SensorInfoBase.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {
    /** Specific implementation of SensorInfo for SVD Sensors which provides additional sensor specific information.
     * This is a temporary fixup: the parameters are stored in the geometry xml files.
     */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** Enum for parametric access to sensor coordinates. */
      enum Coordinate {
        u = 0,
        v = 1
      };
      // FIXME: Consolidate with similar enums in the digitizer.
      /** Constructor which automatically sets the SensorType to SensorInfo::SVD.
       * @param id VXD ID of the sensor.
       * @width Width of the sensor.
       * @length Length of the sensor.
       * @thickness Thickness of the senosr.
       * @uCells Number of strips in u dirrection.
       * @vCells Number of strips in v direction.
       * @width2 For wedge sensors, width is the width at 0, width2 is the width at maximum u.
       */
      SensorInfo(VxdID id = 0, float width = 0, float length = 0, float thickness = 0,
                 int uCells = 0, int vCells = 0, float width2 = 0):
        VXD::SensorInfoBase(SensorInfo::SVD, id, width, length, thickness, uCells, vCells, width2, -1, 0) {}

      /** Change the SensorID. Useful to copy the SensorInfo from one sensor and use it for another.
       * @param id VxdID to be assigned to current sensor.
       */
      void setID(VxdID id) { m_id = id; }

      /** Set sensor operation parameters.
       * @param depletionVoltage Depletion voltage of the sensor.
       * @param biasVoltage Bias voltage on the sensor.
       * @param backplaneCapacitance Backplane capacitance wrt. the strips.
       * @param interstripCapacitance Interstrip capacitance for the sensor.
       * @param coupling capacitance Coupling capacitance for the strips.
       */
      void setSensorParams(double depletionVoltage, double biasVoltage,
                           double backplaneCapacitance, double interstripCapacitance, double couplingCapacitance) {
        m_depletionVoltage = depletionVoltage;
        m_biasVoltage = biasVoltage;
        m_backplaneCapacitance = backplaneCapacitance;
        m_interstripCapacitance = interstripCapacitance;
        m_couplingCapacitance = couplingCapacitance;
      }

      /** Return the depletion voltage of the sensor. */
      double getDepletionVoltage() const { return m_depletionVoltage; }
      /** Return the bias voltage on the sensor. */
      double getBiasVoltage() const { return m_biasVoltage; }
      /** Return the backplane capacitance for the sensor. */
      double getBackplaneCapacitance() const { return m_backplaneCapacitance; }
      /** Return the interstrip capacitance for the sensor. */
      double getInterstripCapacitance() const { return m_interstripCapacitance; }
      /** Return the coupling capacitance of the sensor strips */
      double getCouplingCapacitance() const { return m_couplingCapacitance; }
      /** Return the zero suppression cut for the sensor. */

    protected:
      /** The depletion voltage of the Silicon sensor */
      double m_depletionVoltage;
      /** The bias voltage on the sensor */
      double m_biasVoltage;
      /** The backplane capacitance wrt. the strips. */
      double m_backplaneCapacitance;
      /** The interstrip capacitance for the sensor. */
      double m_interstripCapacitance;
      /** The coupling capacitance for the sensor. */
      double m_couplingCapacitance;
    }; // Class SVD::SensorInfo

  } // SVD namespace
} //Belle2 namespace
#endif
