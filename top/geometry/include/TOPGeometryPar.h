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
#include <string>

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
       * Adds number to string
       * @param str string
       * @param number number to be added
       * @return string with a number
       */
      std::string addNumber(const std::string& str, unsigned number);

      /**
       * Constructs envelope of quantum efficiency from PMT data
       */
      void setEnvelopeQE();

      // Geometry

      TOPGeometry* m_geo = 0;             /**< geometry parameters from Gearbox */
      DBObjPtr<TOPGeometry>* m_geoDB = 0; /**< geometry parameters from database */
      bool m_fromDB = false;              /**< parameters from database or Gearbox */
      bool m_valid = false;               /**< true if geometry is available */

      // Mappings

      FrontEndMapper m_frontEndMapper; /**< front end electronics mapper */
      ChannelMapper m_channelMapperIRS3B; /**< channel-pixel mapper */
      ChannelMapper m_channelMapperIRSX;  /**< channel-pixel mapper */

      // PMT data
      DBArray<TOPPmtInstallation> m_pmtInstalled; /**< PMT installation data */
      DBArray<TOPPmtQE> m_pmtQEData; /**< quantum efficiencies */
      mutable TOPNominalQE m_envelopeQE;  /**< envelope quantum efficiency */

      // Other

      static TOPGeometryPar* s_instance;  /**< Pointer to the class instance */

    };

  } // end of namespace TOP
} // end of namespace Belle2
