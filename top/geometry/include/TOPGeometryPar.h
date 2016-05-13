/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeometry.h>
#include <top/geometry/FrontEndMapper.h>
#include <top/geometry/ChannelMapper.h>

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
      virtual ~TOPGeometryPar() {s_instance = 0;}

      /**
       * Static method to obtain a pointer to its instance.
       * @return pointer to the instance of this class.
       */
      static TOPGeometryPar* Instance();

      /**
       * Initialize from Gearbox (only QBB parameters and mappings)
       * This function will be removed in future!
       * @param content GearBox directory where .xml are stored
       */
      void Initialize(const GearDir& content);

      /**
       * check if the parameters are read from Gearbox
       * @return true if done
       */
      bool isInitialized() const {return m_initialized;}

      /**
       * Sets a pointer to geometry object
       * @param geo pointer to geometry object
       */
      void setGeometry(const TOPGeometry* geo) {m_geo = geo;}

      /**
       * to return parameters in the default units
       */
      void setBasfUnits() {m_unit = Unit::cm;}

      /**
       * to return parameters in Gean units
       */
      void setGeanUnits() {m_unit = Unit::mm;}

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
       * Returns front-end mapper (mapping of SCROD's to positions within TOP modules)
       * @return  front-end mapper object
       */
      const FrontEndMapper& getFrontEndMapper() const {return m_frontEndMapper;}

      /**
       * Returns channel mapper (mapping of channels to pixels)
       * @return  channel mapper object
       */
      const ChannelMapper& getChannelMapper(ChannelMapper::EType type =
                                              ChannelMapper::c_IRSX) const
      {
        switch (type) {
          case ChannelMapper::c_IRS3B: return m_channelMapperIRS3B;
          case ChannelMapper::c_IRSX:  return m_channelMapperIRSX;
          default: return m_channelMapperIRSX;
        }
      }

      /**
       * Returns pointer to geometry object using Basf2 units
       * @return pointer to geometry object
       */
      const TOPGeometry* getGeometry() const
      {
        TOPGeometry::useBasf2Units();
        if (m_geo) return m_geo;
        B2FATAL("No geometry available for TOP");
        return 0;
      }

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
       * Reads geometry parameters from gearbox
       * @param content GearBox directory where .xml are stored
       */
      void read(const GearDir& content);

      // Support structure parameters

      double m_PannelThickness = 0;   /**< thickness of QBB pannel */
      double m_PlateThickness = 0;    /**< thickness of side pladte */
      double m_LowerGap = 0;          /**< lower air gap */
      double m_UpperGap = 0;          /**< upper air gap */
      double m_SideGap = 0;           /**< side air gap */
      double m_forwardGap = 0;        /**< forward air gap */
      double m_backwardGap = 0;       /**< backward air gap */
      std::string m_pannelMaterial;   /**< pannel material name */
      std::string m_insideMaterial;   /**< inside box material name */

      // Mappings

      FrontEndMapper m_frontEndMapper; /**< front end electronics mapper */
      ChannelMapper m_channelMapperIRS3B; /**< channel-pixel mapper */
      ChannelMapper m_channelMapperIRSX;  /**< channel-pixel mapper */

      // Other

      double m_unit = Unit::cm;       /**< conversion unit for length */
      bool m_initialized = false;     /**< true if Initialize() called */
      double m_brokenFraction = 0;    /**< broken glue joint fraction */

      const TOPGeometry* m_geo = 0;   /**< geometry parameters */

      static TOPGeometryPar* s_instance;  /**< Pointer to class instance */

    };

  } // end of namespace TOP
} // end of namespace Belle2
