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
#include <TVector3.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the SVD */
  namespace SVD {
    /** Specific implementation of SensorInfo for SVD Sensors which provides additional sensor specific information.
     * This is a temporary fixup: the parameters are stored in the geometry xml files.
     */
    class SensorInfo: public VXD::SensorInfoBase {
    public:
      /** The Fano factor for silicon. */
      const double c_fanoFactorSi = 0.08;

      /** Enum for parametric access to sensor coordinates. */
      enum Coordinate {
        u = 1,
        v = 0
      };
      /**
       * Enum to flag charge carriers.
       */
      enum CarrierType {
        electron = -1, /** electrons */
        hole = +1      /** holes */
      };

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
        VXD::SensorInfoBase(SensorInfo::SVD, id, width, length, thickness, uCells, vCells, width2, -1, 0),
        m_temperature(300), m_stripEdgeU(0), m_stripEdgeV(0),
        m_depletionVoltage(0), m_biasVoltage(0),
        m_backplaneCapacitanceU(0), m_interstripCapacitanceU(0),
        m_couplingCapacitanceU(0), m_backplaneCapacitanceV(0),
        m_interstripCapacitanceV(0), m_couplingCapacitanceV(0),
        m_aduEquivalentU(375), m_aduEquivalentV(375),
        m_electronicNoiseU(0), m_electronicNoiseV(0),
        m_aduEquivalentSbwU(375), m_aduEquivalentSbwV(375),
        m_electronicNoiseSbwU(0), m_electronicNoiseSbwV(0),
        m_isBackward(false)
      {
        setIsBackward(id);
      }

      /** Determine if this is a backward side barrel sensor.
       * We have special noise settings for these snesors.
       * We need to cheat here, since we don't want to ask the GeoCache.
       * @ param id VxdID of this sensor
       */
      void setIsBackward(VxdID id)
      {
        static const unsigned short layerSensors[] = {0, 0, 0, 2, 3, 4, 5};
        m_isBackward =
          (id.getLayerNumber() > 3)
          && (id.getSensorNumber() == layerSensors[id.getLayerNumber()]);
      }

      /** Change the SensorID. Useful to copy the SensorInfo from one sensor and use it for another.
       * @param id VxdID to be assigned to current sensor.
       */
      void setID(VxdID id)
      {
        m_id = id;
        setIsBackward(id);
      }

      /** Set sensor operation parameters.
       * @param stripEdgeU distance from end of strip to edge of active area.
       * @param stripEdgeV distance from end of strip to edge of active area.
       * @param depletionVoltage Depletion voltage of the sensor.
       * @param biasVoltage Bias voltage on the sensor.
       * @param backplaneCapacitance Backplane capacitance wrt. the strips.
       * @param interstripCapacitance Interstrip capacitance for the sensor.
       * @param coupling capacitance Coupling capacitance for the strips.
       */
      void setSensorParams(double stripEdgeU, double stripEdgeV,
                           double depletionVoltage, double biasVoltage,
                           double backplaneCapacitanceU,
                           double interstripCapacitanceU,
                           double couplingCapacitanceU,
                           double backplaneCapacitanceV,
                           double interstripCapacitanceV,
                           double couplingCapacitanceV,
                           double AduEquivalentU,
                           double AduEquivalentV,
                           double electronicNoiseU,
                           double electronicNoiseV,
                           double AduEquivalentSbwU,
                           double AduEquivalentSbwV,
                           double electronicNoiseSbwU,
                           double electronicNoiseSbwV)
      {
        m_stripEdgeU = stripEdgeU,
        m_stripEdgeV = stripEdgeV,
        m_depletionVoltage = depletionVoltage;
        m_biasVoltage = biasVoltage;
        m_backplaneCapacitanceU = backplaneCapacitanceU;
        m_interstripCapacitanceU = interstripCapacitanceU;
        m_couplingCapacitanceU = couplingCapacitanceU;
        m_backplaneCapacitanceV = backplaneCapacitanceV;
        m_interstripCapacitanceV = interstripCapacitanceV;
        m_couplingCapacitanceV = couplingCapacitanceV;
        m_aduEquivalentU = AduEquivalentU;
        m_aduEquivalentV = AduEquivalentV;
        m_electronicNoiseU = electronicNoiseU;
        m_electronicNoiseV = electronicNoiseV;
        m_aduEquivalentSbwU = AduEquivalentSbwU;
        m_aduEquivalentSbwV = AduEquivalentSbwV;
        m_electronicNoiseSbwU = electronicNoiseSbwU;
        m_electronicNoiseSbwV = electronicNoiseSbwV;
      }

      /** Return the sensor temperature.*/
      double getTemperature() const {return m_temperature; }
      /** Return u-strip length.
       * @param uID number of the strip (ignored for rectangular sensors)
       * @return length of the u-strip.
       */
      double getStripLengthU(int uID = 0) const;
      /** Return v-strip length.
       * @param vID number of the strip (ignored for rectangular sensors)
       * @return length of the v-strip.
       */
      double getStripLengthV(int vID = 0) const;
      /** Return the depletion voltage of the sensor. */
      double getDepletionVoltage() const { return m_depletionVoltage; }
      /** Return the bias voltage on the sensor. */
      double getBiasVoltage() const { return m_biasVoltage; }
      /** Return the backplane capacitanceU for the sensor's u strips (long). */
      double getBackplaneCapacitanceU(int uID = 0) const;
      /** Return the interstrip capacitanceU for the sensor's u strips (long). */
      double getInterstripCapacitanceU(int uID = 0) const;
      /** Return the coupling capacitanceU of the sensor's u strips (long) */
      double getCouplingCapacitanceU(int uID = 0) const;
      /** Return the backplane capacitanceV for the sensor's v strips (short). */
      double getBackplaneCapacitanceV(int vID = 0) const;
      /** Return the interstrip capacitanceV for the sensor's v strips (short). */
      double getInterstripCapacitanceV(int vID = 0) const;
      /** Return the coupling capacitanceV of the sensor's v strips (short). */
      double getCouplingCapacitanceV(int vID = 0) const;
      /** Return ADU equivalent for u strips */
      double getAduEquivalentU() const;
      /** Return ADU equivalent for v strips */
      double getAduEquivalentV() const;
      /** Return electronic noise in e- for u (long) strips */
      double getElectronicNoiseU() const;
      /** Return electronic noise in e- for v (short) strips */
      double getElectronicNoiseV() const;
      /** Calculate electron mobility at a given electric field.
       * Based on C. Canali et al., IEEE, ED-22, (1975) 1045
       * @param eField Electric field, V/cm
       * @return electron mobility, cm*2/V.ns
       */
      double getElectronMobility(double E) const;
      /** Calculate hole mobility at a given electric field.
       * Based on C. Canali et al., IEEE, ED-22, (1975) 1045
       * @param eField Electric field, V/cm
       * @return hole mobility, cm*2/V.ns
       */
      double getHoleMobility(double E) const;

      /** Model of the E field inside the sensor.
       * @param point Desired position in local coordinates.
       * @return The E field vector in local coordinates.
       */
      const TVector3 getEField(const TVector3& point) const;

      /** Get B field value from the field map.
       * @param point Desired position in local coordinates.
       * @return The B field vector in local coordinates.
       */
      const TVector3& getBField(const TVector3& point) const;

      /** Return Hall factor for the corresponding carrier type.
       * @param carrier electron or hole, SVD::SensorInfo::CarrierType
       * @return The Hall factor for the actual sensor temperature.
       */
      double getHallFactor(CarrierType carrier) const
      {
        if (carrier == electron)
          return (1.13 + 0.0008 * (m_temperature - 273));
        else
          return (0.72 - 0.0005 * (m_temperature - 273));
      }

      /** Get drift velocity for electrons or holes at a given point.
       * @param carrier Electron or hole.
       * @param point The point in local coordinates.
       * @result The vector of drift velocity in local coordinates.
       */
      const TVector3 getVelocity(CarrierType carrier, const TVector3& point) const;

      /** Calculate Lorentz shift along a given coordinate in a magnetic field at a given position.
        * This method can only be used for a completely reconstructed 2D cluster.
        * For 1D clusters, use the following method.
        * @param uCoord u coordinate where the shift is required
        * @param vCoord v coordinate where the shift is required
        * @return TVector with Lorentz shift along u and v at the given position.
        */
      const TVector3& getLorentzShift(double uCoord, double vCoord) const;

      /** Calculate mean Lorentz shift along a given coordinate, with B-field averaged
       * along the corresponding strip.
       * Use this for 1D clusters, where only one coordinate is known.
       * @param isUCoordinate True if u, false if v.
       * @param position The position of the strip.
       * @return Mean Lorentz shift along a given coordinate.
       */
      double getLorentzShift(bool isUCoordinate, double position) const;

    protected:
      /** Sensor temperature */
      double m_temperature;
      /** The distance between end of strips and edge of active area */
      double m_stripEdgeU;
      /** The distance between end of strips and edge of active area */
      double m_stripEdgeV;
      /** The depletion voltage of the Silicon sensor */
      double m_depletionVoltage;
      /** The bias voltage on the sensor */
      double m_biasVoltage;
      /** The backplane capacitance/cm for the sensor's u strips */
      double m_backplaneCapacitanceU;
      /** The interstrip capacitance/cm for the sensor's u strips */
      double m_interstripCapacitanceU;
      /** The coupling capacitance/cm for the sensor's u strips */
      double m_couplingCapacitanceU;
      /** The backplane capacitance/cm for the sensor's v strips */
      double m_backplaneCapacitanceV;
      /** The interstrip capacitance/cm for the sensor's v strips. */
      double m_interstripCapacitanceV;
      /** The coupling capacitance/cm for the sensor's v strips. */
      double m_couplingCapacitanceV;
      /** ADU equivalent (electrons/ADU) for u strips */
      double m_aduEquivalentU;
      /** ADU equivalent (electrons/ADU) for v strips */
      double m_aduEquivalentV;
      /** The electronic noise for u (long) strips. */
      double m_electronicNoiseU;
      /** The electronic noise for v (short) strips. */
      double m_electronicNoiseV;
      /** ADU equivalent (electrons/ADU) for u strips, Sbw barrel sensors */
      double m_aduEquivalentSbwU;
      /** ADU equivalent (electrons/ADU) for v strips, Sbw barrel sensors */
      double m_aduEquivalentSbwV;
      /** The electronic noise for u strips in bw barrel (non-Origami) sensors. */
      double m_electronicNoiseSbwU;
      /** The electronic noise for v strips in bw barrel (non-Origami) sensors. */
      double m_electronicNoiseSbwV;
      /** Is this a backward barrel sensor? */
      bool m_isBackward;
    }; // Class SVD::SensorInfo

    inline double SensorInfo::getStripLengthU(int uID) const
    {
      // if this is a rectangular sensor, just return the default
      if (m_deltaWidth == 0.0)
        return (m_length - 2.0 * m_stripEdgeU);
      else {
        // calculate for a wedge sensor
        double dw = (1.0 * uID / m_uCells - 0.5) * m_deltaWidth;
        return (sqrt(dw * dw + m_length * m_length) - 2.0 * m_stripEdgeU);
      }
    }

    inline double SensorInfo::getStripLengthV(int vID) const
    {
      return (getWidth(getVCellPosition(vID)) - 2.0 * m_stripEdgeV);
    }

    inline double SensorInfo::getBackplaneCapacitanceU(int uID) const
    {
      return (m_backplaneCapacitanceU * getStripLengthU(uID));
    }

    inline double SensorInfo::getInterstripCapacitanceU(int uID) const
    {
      return (m_interstripCapacitanceU * getStripLengthU(uID));
    }

    inline double SensorInfo::getCouplingCapacitanceU(int uID) const
    {
      return (m_couplingCapacitanceU * getStripLengthU(uID));
    }

    inline double SensorInfo::getBackplaneCapacitanceV(int vID) const
    {
      return (m_backplaneCapacitanceV * getStripLengthV(vID));
    }

    inline double SensorInfo::getInterstripCapacitanceV(int vID) const
    {
      return (m_interstripCapacitanceV * getStripLengthV(vID));
    }

    inline double SensorInfo::getCouplingCapacitanceV(int vID) const
    {
      return (m_couplingCapacitanceV * getStripLengthV(vID));
    }

    inline double SensorInfo::getAduEquivalentU() const
    {
      if (m_isBackward)
        return m_aduEquivalentSbwU;
      else
        return m_aduEquivalentU;
    }

    inline double SensorInfo::getAduEquivalentV() const
    {
      if (m_isBackward)
        return m_aduEquivalentSbwU;
      else
        return m_aduEquivalentV;
    }

    inline double SensorInfo::getElectronicNoiseU() const
    {
      if (m_isBackward)
        return m_electronicNoiseSbwU;
      else
        return m_electronicNoiseU;
    }

    inline double SensorInfo::getElectronicNoiseV() const
    {
      if (m_isBackward)
        return m_electronicNoiseSbwV;
      else
        return m_electronicNoiseV;
    }


  } // SVD namespace
} //Belle2 namespace
#endif
