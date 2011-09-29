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

    class Digit {
    public:
      Digit(unsigned short u = 0, unsigned short v = 0): m_u(u), m_v(v) {}
      bool operator<(const Digit &b)  const { return v() < b.v() || (v() == b.v() && u() < b.u()); }
      bool operator==(const Digit &b) const { return v() == b.v() && u() == b.u(); }
      unsigned short u() const { return m_u; }
      unsigned short v() const { return m_v; }
    protected:
      unsigned short m_u;
      unsigned short m_v;
    };

    class DigitValue {
    public:
      typedef std::map<RelationElement::index_type, RelationElement::weight_type> relations_map;
      DigitValue(): m_charge(0) {}
      void add(double charge, int particle = -1) {
        if (particle >= 0) m_particles[particle] += charge;
        m_charge += charge;
      }
      double charge() const { return m_charge; }
      const relations_map &relations() const { return m_particles; }
    protected:
      double m_charge;
      relations_map m_particles;
    };



    typedef std::map<Digit, DigitValue> Sensor;
    typedef std::map<unsigned short, Sensor> Sensors;

    /** The PXD Digitizer module. */
    class PXDDigitizerModule : public Module {
    public:
      /**
       * Constructor.
       * Sets the module parameters.
       */
      PXDDigitizerModule();

      void processHit();
      void driftCharge(const TVector3 &position, double electrons);
      void driftChargeSimple(const TVector3 &position, double electrons);
      void addNoiseDigits();
      double addNoise(double charge);
      void saveDigits();

      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void terminate();

    protected:
      bool   m_applyNoise;
      double m_elNoise;
      double m_SNAdjacent;
      double m_noiseFraction;

      std::string m_mcColName;
      std::string m_digitColName;
      std::string m_simhitColName;
      std::string m_relSimName;
      std::string m_relDigitName;

      bool   m_applyPoisson;
      bool   m_applyWindow;

      double m_segmentLength;
      int    m_elGroupSize;
      double m_elStepTime;
      int    m_elMaxSteps;
      double m_tanLorentz;

      bool   m_applyADC;
      double m_rangeADC;
      double m_bitsADC;
      double m_unitADC;

      bool   m_useSimpleDrift;
      double m_widthOfDiffusCloud;
      double m_diffusionCoefficient;

      Sensors m_sensors;
      TRandom* m_random;

      const PXDSimHit*   m_currentHit;
      int                m_currentParticle;
      Sensor*            m_currentSensor;
      const SensorInfo*  m_currentSensorInfo;

      std::string m_rootFilename;
      TFile* m_rootFile;
      TH1D*  m_histSteps;
      TH2D*  m_histDiffusion;

    };//end class declaration
  } // end namespace PXD
} // end namespace Belle2

#endif // PXDDigitizerModule_H
