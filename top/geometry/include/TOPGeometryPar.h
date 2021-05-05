/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 * Major revision: 2016                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/FrontEndMapper.h>
#include <top/geometry/ChannelMapper.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <top/dbobjects/TOPPmtInstallation.h>
#include <top/dbobjects/TOPPmtQE.h>
#include <top/dbobjects/TOPNominalQE.h>
#include <top/dbobjects/TOPCalChannelRQE.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>
#include <string>
#include <map>

namespace Belle2 {
  namespace TOP {

    /**
     * Singleton class for TOP Geometry Parameters.
     */

    class TOPGeometryPar {

    public:

      /**
       * Destructor
       */
      virtual ~TOPGeometryPar();

      /**
       * Static method to obtain the pointer to its instance.
       * @return pointer to the instance of this class.
       */
      static TOPGeometryPar* Instance();

      /**
       * Initialize from Gearbox (XML)
       * @param content XML data directory
       */
      void Initialize(const GearDir& content);

      /**
       * Initialize from database
       */
      void Initialize();

      /**
       * check if the geometry is available
       * @return true if available
       */
      bool isValid() const {return m_valid;}

      /**
       * Returns pointer to geometry object using Basf2 units
       * @return pointer to geometry object
       */
      const TOPGeometry* getGeometry() const;

      /**
       * Returns front-end mapper (mapping of SCROD's to positions within TOP modules)
       * @return  front-end mapper object
       */
      const FrontEndMapper& getFrontEndMapper() const {return m_frontEndMapper;}

      /**
       * Returns default channel mapper (mapping of channels to pixels)
       * @return  channel mapper object
       */
      const ChannelMapper& getChannelMapper() const {return m_channelMapperIRSX;}

      /**
       * Returns channel mapper (mapping of channels to pixels) - Gearbox only
       * @return  channel mapper object
       */
      const ChannelMapper& getChannelMapper(ChannelMapper::EType type) const
      {
        switch (type) {
          case ChannelMapper::c_IRS3B: return m_channelMapperIRS3B;
          case ChannelMapper::c_IRSX:  return m_channelMapperIRSX;
          default: return m_channelMapperIRSX;
        }
      }

      /**
       * Returns PMT efficiency envelope, e.g. at given photon energy the maximum
       * over all PMT's of a product of quantum and collection efficiency.
       * @param energy photon energy in [eV]
       * @return the maximal efficiency
       */
      double getPMTEfficiencyEnvelope(double energy) const;

      /**
       * Returns PMT pixel efficiency, a product of quantum and collection efficiency.
       * @param energy photon energy in [eV]
       * @param moduleID slot ID
       * @param pmtID PMT ID
       * @param x photon detection position x in local PMT frame
       * @param y photon detection position y in local PMT frame
       * @return the efficiency
       */
      double getPMTEfficiency(double energy,
                              int moduleID, int pmtID, double x, double y) const;

      /**
       * Returns relative pixel efficiency (including CE, RQE and threshold efficiency)
       * @return pixel efficiency relative to nominal photocathode
       */
      double getRelativePixelEfficiency(int moduleID, int pixelID) const;

      /**
       * Returns PMT type at a given position
       * @param moduleID slot ID
       * @param pmtID PMT ID
       * @return PMT type
       */
      unsigned getPMTType(int moduleID, int pmtID) const;

      /**
       * Returns TTS of a PMT at given position
       * @param moduleID slot ID
       * @param pmtID PMT ID
       * @return TTS
       */
      const TOPNominalTTS& getTTS(int moduleID, int pmtID) const;

      /**
       * Returns phase refractive index of quartz at given photon energy
       * @param energy photon energy [eV]
       * @return phase refractive index
       */
      double getPhaseIndex(double energy) const;

      /**
       * Returns group refractive index of quartz at given photon energy
       * @param energy photon energy [eV]
       * @return group refractive index
       */
      double getGroupIndex(double energy) const;

      /**
       * Returns the derivative (dn/dE) of phase refractive index of quartz at given photon energy
       * @param energy photon energy [eV]
       * @return derivative of phase refractive index
       */
      double getPhaseIndexDerivative(double energy) const;

      /**
       * Returns the derivative (dn_g/dE) of group refractive index of quartz at given photon energy
       * @param energy photon energy [eV]
       * @return group refractive index
       */
      double getGroupIndexDerivative(double energy) const;

      /**
       * Returns bulk absorption lenght of quartz at given photon energy
       * @param energy photon energy [eV]
       * @return bulk absorption lenght
       */
      double getAbsorptionLength(double energy) const;

      static const double c_hc; /**< Planck constant times speed of light in [eV*nm] */

    private:

      /**
       * Hidden constructor since it is a singleton class
       */
      TOPGeometryPar()
      {}

      /**
       * finalize initialization
       */
      void finalizeInitialization();

      /**
       * Create a parameter object from gearbox
       * @param content XML data directory
       */
      TOPGeometry* createConfiguration(const GearDir& content);

      /**
       * Create a parameter object from gearbox for bar segment
       * @param content XML data directory
       * @param serialNumber bar segment serial number
       */
      TOPGeoBarSegment createBarSegment(const GearDir& content,
                                        const std::string& serialNumber);

      /**
       * Create a parameter object from gearbox for mirror segment
       * @param content XML data directory
       * @param serialNumber mirror segment serial number
       */
      TOPGeoMirrorSegment createMirrorSegment(const GearDir& content,
                                              const std::string& serialNumber);

      /**
       * Create a parameter object from gearbox for prism
       * @param content XML data directory
       * @param serialNumber prism serial number
       */
      TOPGeoPrism createPrism(const GearDir& content,
                              const std::string& serialNumber);

      /**
       * Adds number to string
       * @param str string
       * @param number number to be added
       * @return string with a number
       */
      std::string addNumber(const std::string& str, unsigned number);

      /**
       * Clears cache for PMT dependent QE data - function is used in call backs
       */
      void clearCache();

      /**
       * Constructs envelope of quantum efficiency from PMT data
       */
      void setEnvelopeQE() const;

      /**
       * Maps PMT QE data to positions within the detector
       */
      void mapPmtQEToPositions() const;

      /**
       * Maps PMT type to positions within the detector
       */
      void mapPmtTypeToPositions() const;

      /**
       * Prepares a map of relative pixel efficiencies
       */
      void prepareRelEfficiencies() const;

      /**
       * Returns unique PMT ID within the detector
       * @param moduleID slot ID
       * @param pmtID PMT ID
       * @return unique ID
       */
      int getUniquePmtID(int moduleID, int pmtID) const
      {
        return (moduleID << 16) + pmtID;
      }

      /**
       * Returns unique pixel ID within the detector
       * @param moduleID slot ID
       * @param pixelID pixel ID
       * @return unique ID
       */
      int getUniquePixelID(int moduleID, int pixelID) const
      {
        return (moduleID << 16) + pixelID;
      }

      /**
       * Returns integral of quantum efficiency over photon energies
       * @param qe quantum efficiency data points
       * @param ce collection efficiency data points
       * @param lambdaFirst wavelenght of the first data point [nm]
       * @param lambdaStep wavelength step [nm]
       * @return integral [eV]
       */
      double integralOfQE(const std::vector<float>& qe, double ce,
                          double lambdaFirst, double lambdaStep) const;


      /**
       * Quartz refractive index (SellMeier equation)
       * @param lambda photon wavelength [nm]
       * @return refractive index
       */
      double refractiveIndex(double lambda) const;

      // Geometry

      TOPGeometry* m_geo = 0;             /**< geometry parameters from Gearbox */
      DBObjPtr<TOPGeometry>* m_geoDB = 0; /**< geometry parameters from database */
      bool m_fromDB = false;              /**< parameters from database or Gearbox */
      bool m_valid = false;               /**< true if geometry is available */
      bool m_oldPayload = false;          /**< true if old payload found in DB */
      bool m_BfieldOn =  true;            /**< true if B field is on */

      // Mappings

      FrontEndMapper m_frontEndMapper; /**< front end electronics mapper */
      ChannelMapper m_channelMapperIRS3B; /**< channel-pixel mapper */
      ChannelMapper m_channelMapperIRSX;  /**< channel-pixel mapper */

      // PMT database

      OptionalDBArray<TOPPmtInstallation> m_pmtInstalled; /**< PMT installation data */
      OptionalDBArray<TOPPmtQE> m_pmtQEData; /**< quantum efficiencies */
      DBObjPtr<TOPCalChannelRQE> m_channelRQE; /**< channel relative quantum effi. */
      DBObjPtr<TOPCalChannelThresholdEff> m_thresholdEff; /**< channel threshold effi. */

      // cache
      mutable TOPNominalQE m_envelopeQE;  /**< envelope quantum efficiency */
      mutable std::map<int, const TOPPmtQE*> m_pmts; /**< QE data mapped to positions */
      mutable std::map<int, double> m_relEfficiencies; /**< pixel relative QE */
      mutable std::map<int, unsigned> m_pmtTypes; /**< PMT types mapped to positions */

      // Other

      static TOPGeometryPar* s_instance;  /**< Pointer to the class instance */

    };

    inline double TOPGeometryPar::getPhaseIndexDerivative(double energy) const
    {
      double dE = 0.01; // [eV]
      return (getPhaseIndex(energy + dE / 2) - getPhaseIndex(energy - dE / 2)) / dE;
    }

    inline double TOPGeometryPar::getGroupIndexDerivative(double energy) const
    {
      double dE = 0.01; // [eV]
      return (getGroupIndex(energy + dE / 2) - getGroupIndex(energy - dE / 2)) / dE;
    }

    inline double TOPGeometryPar::getAbsorptionLength(double energy) const
    {
      double lambda = c_hc / energy;
      return 15100 * pow(lambda / 405, 4); // Alan Schwartz, 2013 (private comunication)
    }

  } // end of namespace TOP
} // end of namespace Belle2
