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
#include <framework/gearbox/Unit.h>
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
       * Static method to obtain a pointer to its instance.
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
       * to return parameters in the default units
       */
      void setBasfUnits() {m_unit = Unit::cm;}

      /**
       * to return parameters in Gean units
       */
      void setGeanUnits() {m_unit = Unit::mm;}

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
       * Returns channel mapper (mapping of channels to pixels)
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


      // old QBB definition ---------------------->

      /**
       * Returns thickness of the QBB pannel
       * @return thicness of QBB pannel
       */
      double getPannelThickness() const {return m_PannelThickness / m_unit; }

      /**
       * Returns thickness of the sinde plate
       * @return thickness of side pladte
       */
      double getPlateThickness() const {return m_PlateThickness / m_unit; }

      /**
       * Returns size of air gap between quartz and support at inner radious
       * @return air gar
       */
      double getLowerGap() const {return m_LowerGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support at outer radious
       * @return air gar
       */
      double getUpperGap() const {return m_UpperGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support (side plate)
       * @return air gar
       */
      double getSideGap() const {return m_SideGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support (forward plate)
       * @return air gar
       */
      double getForwardGap() const {return m_forwardGap / m_unit; }

      /**
       * Returns size of air gap between quartz and support (backward plate)
       * @return air gar
       */
      double getBackwardGap() const {return m_backwardGap / m_unit; }

      /**
       * Returns pannel material name
       * @return material name
       */
      const std::string& getPannelMaterial() const {return m_pannelMaterial;}

      /**
       * Returns inside box material name
       * @return material name
       */
      const std::string& getInsideMaterial() const {return m_insideMaterial;}

      // end old QBB definition <--------------------------------------

      /**
       * Returns broken glue joint fraction
       * @return broken glue joint fraction
       */
      double getBrokenFraction() const {return m_brokenFraction;}


    private:

      /**
       * Hidden constructor since it is a singleton class
       */
      TOPGeometryPar()
      {}

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
       * Reads old QBB from gearbox
       * @param content XML data directory
       */
      void readOldQBB(const GearDir& content);

      // Geometry

      TOPGeometry* m_geo = 0;             /**< geometry parameters from Gearbox */
      DBObjPtr<TOPGeometry>* m_geoDB = 0; /**< geometry parameters from database */
      bool m_fromDB = false;              /**< parameters from database or Gearbox */
      bool m_valid = false;               /**< true if geometry is available */

      // Old QBB parameters (be removed...)

      double m_PannelThickness = 1.0;   /**< thickness of QBB pannel */
      double m_PlateThickness = 0.1;    /**< thickness of side pladte */
      double m_LowerGap = 0.2;          /**< lower air gap */
      double m_UpperGap = 0.2;          /**< upper air gap */
      double m_SideGap = 0.1;           /**< side air gap */
      double m_forwardGap = 1.0;        /**< forward air gap */
      double m_backwardGap = 12.0;       /**< backward air gap */
      std::string m_pannelMaterial = "TOPQBB";   /**< pannel material name */
      std::string m_insideMaterial = "TOPAir";   /**< inside box material name */

      // Mappings

      FrontEndMapper m_frontEndMapper; /**< front end electronics mapper */
      ChannelMapper m_channelMapperIRS3B; /**< channel-pixel mapper */
      ChannelMapper m_channelMapperIRSX;  /**< channel-pixel mapper */

      // Other

      double m_unit = Unit::cm;       /**< conversion unit for length */
      double m_brokenFraction = 0;    /**< broken glue joint fraction */

      static TOPGeometryPar* s_instance;  /**< Pointer to class instance */

    };

  } // end of namespace TOP
} // end of namespace Belle2
