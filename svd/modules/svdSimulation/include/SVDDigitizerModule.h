/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Zbynek Drasal, Martin Ritter, Peter Kvasnicka            *
 *                                                                        *
 **************************************************************************/

#ifndef SVDDigitizerModule_H
#define SVDDigitizerModule_H

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/dataobjects/RelationElement.h>
#include <string>
#include <set>
#include <vector>
//#include <boost/array.hpp>

#include <root/TVector3.h>
#include <root/TFile.h>
#include <root/TH1D.h>
#include <root/TH2D.h>

namespace Belle2 {
  namespace SVD {

    /**
     * Enum to flag R-Phi vs. Z strips.
     */
    enum StripDirection {
      stripRPhi = 0, /** for strips in R-Phi */
      stripZ = 1     /** for strips in Z     */
    };
    enum CarrierType {
      electron = -1,
      hole = +1
    };

    // FIXME: Has to go to Unit.h
    const double FanoFactorSi = 0.08;

    /** Map of all signals in one sensor. */
    typedef std::map<short int, SVDSignal> StripSignals;

    /** Signals of u- and v- strips in one sensor. */
    typedef std::pair<StripSignals, StripSignals> Sensor;

    /** Map of all signals in all sensors */
    typedef std::map<VxdID, Sensor> Sensors;


    /** The SVD Digitizer module. */

    class SVDDigitizerModule : public Module {
    public:
      /** Constructor.  */
      SVDDigitizerModule();

      /** Process one SVDSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();

      /** Calculate electron mobility at a given electric field.
       * @param eField Electric field, V/cm
       * @return electron mobility, cm*2/V.ns
       */
      double getElectronMobility(double E) const;

      /** Calculate hole mobility at a given electric field.
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
      const TVector3 getBField(const TVector3& point) const;

      /** Get drift velocity for electrons or holes at a given point.
       * @param carrier Electron or hole.
       * @param point The point in local coordinates.
       * @result The vector of drift velocity in local coordinates.
       */
      const TVector3 getVelocity(CarrierType carrier, const TVector3& point) const;

      /** Drift the charge inside the silicon.
       * This method will drift the charge inside the silicon along the E/B fieldlines.
       * @param position start position of the charge
       * @param electrons number of electrons and holes to drift
       */
      void driftCharge(const TVector3& position, double carriers);

      /** Calculate the noise contribution to one strip with given charge.
       * @return the new charge of the strip.
       */
      double addNoise(double charge);

      /** Save digits to the DataStore
       * @param time The time when signal sample should be taken and stored.
       */
      void saveDigits(double time);

      /** Initialize the module and check module parameters */
      virtual void initialize();
      /** Initialize the list of existing SVD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:

      // Members holding module parameters:

      // 1. Collections
      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection for the SVDSimhits */
      std::string m_storeSimHitsName;
      /** Name of the collection for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the relation between MCParticles and SVDSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDTrueHits and SVDSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;

      // 2. Physics
      /** Sensor operating temperature in Kelvins. */
      double m_temperature;
      /** Max. Segment length to use for charge drifting */
      double m_segmentLength;
      /** Width of diffusion cloud for simple drift model (in sigmas) */
      double m_widthOfDiffusCloud;

      // 3. Noise
      /** Whether or not to apply poisson fluctuation of charge */
      bool   m_applyPoisson;
      /** Whether or not to apply Gaussian noise */
      bool  m_applyNoise;
      /** Electronic noise. */
      double m_elNoise;
      /** Zero-suppression cut. */
      double m_SNAdjacent;
      /** (derived from SNAdjacent) Fraction of noisy strips per sensor. */
      double m_noiseFraction;

      // 4. Timing
      /** Shaping time of the APV25 shapers.*/
      double m_shapingTime;
      /** Interval between two waveform samples (30 ns). */
      double m_samplingTime;
      /** Whether or not to apply a time window cut */
      bool   m_applyWindow;
      /** Time window start.
       * Starting from this time, signal samples are taken in samplingTime intervals.
       */
      double m_startSampling;
      /** Time window end.
       * Starting from this time, samples are not taken.
       */
      double m_stopSampling;
      /** Whether or not to apply random phase sampling.
       * If set to true, the first samples of the event will be taken at a random time point
       * with probability centered around the time when first particle reaches
       * the SVD. */
      bool m_randomPhaseSampling;

      /** FIXME: There are other options that are not introduced in this version,
       * like different sampling times in different sensors etc.
       */

      // 5. Processing
      /** Whether or not to apply discrete ADC on output values. */
      bool   m_applyADC;
      /** Low end of ADC range in e-. */
      double m_minADC;
      /** High end of ADC range in e-. */
      double m_maxADC;
      /** Number of ADC bits. */
      int m_bitsADC;
      // Derived from above: adu in electrons. */
      double m_unitADC;

      // 6. Reporting
      /** Name of the ROOT filename to output statistics */
      std::string m_rootFilename;

      // Other data members:

      /** Structure containing signals in all existing sensors */
      Sensors m_sensors;

      /** Pointer to the SVDSimhit currently digitized */
      const SVDSimHit*   m_currentHit;
      /** Index of the particle which caused the current hit */
      RelationElement::index_type  m_currentParticle;
      /** Index of the TrueHit the current hit belongs to */
      RelationElement::index_type  m_currentTrueHit;
      /** Pointer to the sensor in which the current hit occurred */
      Sensor*            m_currentSensor;
      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo;
      /** Time of the current detector event, from the currently processed SimHit.. */
      double m_currentTime;

      // Read from m_currentSensorInfo
      double m_sensorThickness;
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

      // ROOT stuff:
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile;
      /** Histogram showing the diffusion cloud */
      TH1D*  m_histDiffusion_u;
      TH1D*  m_histDiffusion_v;

    };//end class declaration
  } // end namespace SVD
} // end namespace Belle2

#endif // SVDDigitizerModule_H
