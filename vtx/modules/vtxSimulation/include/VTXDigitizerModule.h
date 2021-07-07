/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vtx/dataobjects/VTXSimHit.h>
#include <vtx/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TRandom.h>
#include <string>
#include <set>
#include <vector>
#include <boost/array.hpp>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the VTX */
  namespace VTX {

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
      void add(double charge, int particle = -1, int truehit = -1)
      {
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


    /** The VTX Digitizer module.
     * This module is responsible for converting the simulated energy
     * deposition from Geant4 into real VTX detector response of single pixles.
     * This is a much simplified version from the PXDDigitizer.
     * TODO: Include Lorentz shift
     * TODO: Not full thickness is depleted
     * TODO: Charge sharing by diffusion
     */
    class VTXDigitizerModule : public Module {
    public:
      /** Constructor.  */
      VTXDigitizerModule();

      /** Process one VTXSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();
      /** Drift the charge inside the silicon.
       * This method will drift the charge inside the silicon in two steps:
       * 1. Drift to the potential minimum plane by using vertical drift along the fieldlines
       * 2. Random walk inside the potential minimum until approaching an
       *    internal gate close enough to be caught.
       * @param position start position of the charge
       * @param electrons number of electrons to drift
       */
      void driftCharge(const TVector3& position, double electrons);
      /** Add pure noise digits to the Sensors */
      void addNoiseDigits();
      /** Calculate the noise contribution to one pixel with given charge.
       * @return the new charge of the pixel.
       */
      double addNoise(const SensorInfo& info, double charge);
      /** Save all digits to the datastore */
      void saveDigits();



      /** Initialize the module and check the parameters */
      void initialize() override final;
      /** Initialize the list of existing VTX Sensors */
      void beginRun() override final;
      /** Digitize one event */
      void event() override final;

    protected:
      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the VTXDigits */
      std::string m_storeDigitsName;
      /** Name of the collection for the VTXSimhits */
      std::string m_storeSimHitsName;
      /** Name of the collection for the VTXTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the relation between MCParticles and VTXSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between VTXDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between VTXTrueHits and VTXSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between VTXDigits and VTXTrueHits */
      std::string m_relDigitTrueHitName;

      /** Structure containing all existing sensors */
      Sensors m_sensors;

      /** Pointer to the VTXSimhit currently digitized */
      const VTXSimHit*   m_currentHit = nullptr;
      /** Index of the particle which caused the current hit */
      int                m_currentParticle = -1;
      /** Index of the TrueHit the current hit belongs to */
      int                m_currentTrueHit = -1;
      /** Pointer to the sensor in which the current hit occured */
      Sensor*            m_currentSensor = nullptr;
      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo = nullptr;
      /** Current magnetic field */
      TVector3 m_currentBField;

    };//end class declaration


  } // end namespace VTX
} // end namespace Belle2

