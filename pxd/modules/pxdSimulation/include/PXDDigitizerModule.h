/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef PXDDigitizerModule_H
#define PXDDigitizerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/geometry/SensorInfo.h>
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
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {

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


    /** The PXD Digitizer module.
     * This module is responsible for converting the simulated energy
     * deposition from Geant4 into real PXD detector response of single pixles.
     */
    class PXDDigitizerModule : public Module {
    public:
      /** Constructor.  */
      PXDDigitizerModule();

      /** Process one PXDSimHit by dividing the step in smaller steps and drifting the charge */
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
      /** Drift the charge inside the silicon with a simpler approach.
       * The charge is drifted to the sensor surface with gaussian diffusion and the
       * fractions of charge are distributed among the hit pixels. Effective
       * diffusion constant and mean Lorentz angle are used.
       * @param position start position of the charge
       * @param electrons number of electrons to drift
       */
      void driftChargeSimple(const TVector3& position, double electrons);
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
      /** Initialize the list of existing PXD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:
      /** Wether or not to apply noise */
      bool   m_applyNoise;
      /** Amount of noise to apply */
      double m_elNoise;
      /** Signal to Noise ratio for zero suppression cut */
      double m_SNAdjacent;
      /** Fraction of noisy pixels per sensor */
      double m_noiseFraction;

      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the PXDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection for the PXDSimhits */
      std::string m_storeSimHitsName;
      /** Name of the collection for the PXDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the relation between MCParticles and PXDSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between PXDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between PXDTrueHits and PXDSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between PXDDigits and PXDTrueHits */
      std::string m_relDigitTrueHitName;


      /** Wether or not to apply poission fluctuation of charge */
      bool   m_applyPoisson;
      /** Wether or not to apply a time window cut */
      bool   m_applyWindow;

      /** Max. Segment length to use for charge drifting */
      double m_segmentLength;
      /** Max number of electrons per random walk */
      int    m_elGroupSize;
      /** Timeframe for one random walk step */
      double m_elStepTime;
      /** Maximum number of random walks before abort */
      int    m_elMaxSteps;

      /** Whether or not to apply discrete ADC */
      bool   m_applyADC;
      /** ENC equivalent of 1 ADU */
      double m_eToADU;
      /** g_q of a pixel in nA/electrons.*/
      double m_gq;
      /** Is ADC working in fine (hi-res) mode? */
      bool m_ADCFineMode;
      /** Zero-suppression threshold in ADU, steerable */
      double m_chargeThreshold;
      /** ... and its equivalent in electrons */
      double m_chargeThresholdElectrons;
      /** Structure containing all existing sensors */
      Sensors m_sensors;

      /** Pointer to the PXDSimhit currently digitized */
      const PXDSimHit*   m_currentHit;
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

      /** Name of the ROOT filename to output statistics */
      std::string m_rootFilename;
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile;
      /** Histogram showing the number of random steps */
      TH1D*  m_histSteps;
      /** Histogram showing the diffusion cloud */
      TH2D*  m_histDiffusion;
      /** Histogram showing the Lorentz angles in u (r-phi). */
      TH1D*  m_histLorentz_u;
      /** Histogram showing the Lorentz angles in v (z). */
      TH1D*  m_histLorentz_v;


    };//end class declaration


  } // end namespace PXD
} // end namespace Belle2

#endif // PXDDigitizerModule_H
