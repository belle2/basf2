/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef TelDigitizerModule_H
#define TelDigitizerModule_H

#include <framework/core/Module.h>
#include <testbeam/vxd/dataobjects/TelSimHit.h>
#include <testbeam/vxd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <TRandom.h>
#include <string>
#include <set>
#include <vector>
#include <boost/array.hpp>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of EUDET telescopes */
  namespace TEL {

    /** Class to represent the coordinates of one pixel */
    class Digit {
    public:
      /** Constructor */
      Digit(unsigned short u = 0, unsigned short v = 0): m_u(u), m_v(v) {}
      /** Comparison operator to provide unique ordering */
      bool operator<(const Digit& b)  const { return v() < b.v() || (v() == b.v() && u() < b.u()); }
      /** Equality operator to check for equality */
      bool operator==(const Digit& b) const { return v() == b.v() && u() == b.u(); }
      /** Return u (column) ID */
      unsigned short u() const { return m_u; }
      /** Return v (row) ID */
      unsigned short v() const { return m_v; }
    protected:
      /** u (column) ID */
      unsigned short m_u;
      /** v (row) ID */
      unsigned short m_v;
    };

    /** Class representing the charge and particle contributions for one pixel */
    class DigitValue {
    public:
      /** Type to store contributions to pixel charge by different particles */
      typedef std::map<RelationElement::index_type, RelationElement::weight_type> relations_map;
      /** Constructor */
      DigitValue(): m_charge(0) {}
      /** Add charge to the pixel
       * @param charge charge in electrons to be added
       * @param particle Index of the particle contributing the charge, -1 for no particle/noise
       */
      void add(double charge, int particle = -1, int truehit = -1) {
        if (particle >= 0) m_particles[particle] += charge;
        if (truehit >= 0) m_truehits[truehit] += charge;
        m_charge += charge;
      }
      /** Return the charge collected in the pixel */
      double charge() const { return m_charge; }
      /** Return the map containing all particle contributions to the pixel charge */
      const relations_map& particles() const { return m_particles; }
      /** Return the map containing all truehit contributions to the pixel charge */
      const relations_map& truehits() const { return m_truehits; }
    protected:
      /** charge of the pixel */
      double m_charge;
      /** particle contributions to the charge */
      relations_map m_particles;
      /** truehit contributions to the charge */
      relations_map m_truehits;
    };

    /** Map of all hits in one Sensor */
    typedef std::map<Digit, DigitValue> Sensor;
    /** Map of all hits in all Sensors */
    typedef std::map<VxdID, Sensor> Sensors;

    /** \addtogroup modules
     * @{
     */

    /** The Telescope Digitizer module.
     * This module is responsible for converting the simulated energy
     * deposition from Geant4 into binary response of single pixles.
     */
    class TelDigitizerModule : public Module {
    public:
      /** Constructor.  */
      TelDigitizerModule();

      /** Process one TelSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();
      /** Calculate electron mobility at a given electric field.
       * @param eField Electric field, V/cm
       * @return electron mobility, cm*2/V.ns
       */
      double getElectronMobility(double E) const;
      /** Model of the E field inside the sensor.
       * @param point Desired position in local coordinates.
       * @return The E field vector in local coordinates.
       */
      const TVector3 getEField(const TVector3& point) const;

      /** Get B field value from the field map.
       * @param point Desired position in local coordinates.
       * @return The B field vector in local coordinates.
       */
      const TVector3 getBField(const TVector3& point) const;
      /** Get drift velocity for electrons at a given point.
       * The method assumes that the E-field causes the drift that makes electrons
       * susceptible to Lorentz force.
       * @param E Electric field at the desired position
       * @param B Magnetic field at the desired position
       * @result The vector of drift velocity in local coordinates.
       */
      const TVector3 getDriftVelocity(const TVector3& E, const TVector3& B) const;
      /** Drift the charge inside the silicon.
       * The charge is drifted to the sensor surface with gaussian diffusion and the
       * fractions of charge are distributed among the hit pixels. Effective
       * diffusion constant and mean Lorentz angle are used.
       * @param position start position of the charge
       * @param electrons number of electrons to drift
       */
      void driftCharge(const TVector3& position, double electrons);
      /** Add pure noise digits to the Sensors */
      void addNoiseDigits();
      /** Calculate the noise contribution to one pixel with given charge.
       * @return the new charge of the pixel.
       */
      double addNoise(double charge);
      /** Save all digits to the datastore */
      void saveDigits();
      /** Initialize the module and check the parameters */
      virtual void initialize();
      /** Initialize the list of existing Tel Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:
      /** Amount of noise to apply */
      double m_elNoise;
      /** Signal to Noise ratio for zero suppression cut */
      double m_SNThreshold;
      /** Fraction of noisy pixels per sensor */
      double m_noiseFraction;

      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the TelDigits */
      std::string m_storeDigitsName;
      /** Name of the collection for the TelSimhits */
      std::string m_storeSimHitsName;
      /** Name of the collection for the TelTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the relation between MCParticles and TelSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between TelDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between TelTrueHits and TelSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between TelDigits and TelTrueHits */
      std::string m_relDigitTrueHitName;

      /** Max. Segment length to use for charge drifting */
      double m_segmentLength;

      /** Width of diffusion cloud for simple drift model */
      double m_widthOfDiffusCloud;
      /** Diffusion coefficient for simple drift model */
      double m_diffusionCoefficient;
      /** Tangent of the Lorentz angle for simple drift model */
      double m_tanLorentz;

      /** Structure containing all existing sensors */
      Sensors m_sensors;

      /** Pointer to the TelSimhit currently digitized */
      const TelSimHit*   m_currentHit;
      /** Index of the particle which caused the current hit */
      int                m_currentParticle;
      /** Index of the TrueHit the current hit belongs to */
      int                m_currentTrueHit;
      /** Pointer to the sensor in which the current hit occured */
      Sensor*            m_currentSensor;
      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo;
      /** Current magnetic field */
      TVector3 m_currentBField;

    };//end class declaration

    /** @}*/

  } // end namespace TEL
} // end namespace Belle2

#endif // TelDigitizerModule_H
