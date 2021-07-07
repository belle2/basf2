/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeoBKLMCreator.h>
#include <klm/eklm/geometry/GeoEKLMCreator.h>

/**
 * @file
 * KLM geometry crea.
 */

namespace Belle2 {

  namespace KLM {

    /**
     * The creator for the KLM geometry of the Belle II detector.
     */
    class GeoKLMCreator : public geometry::CreatorBase {

    public:

      /**
       * Constructor.
       */
      GeoKLMCreator();

      /**
       * Destructor.
       */
      ~GeoKLMCreator();

      /**
       * Creation of the detector geometry from Gearbox (XML).
       * @param[in] content   XML data directory.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      void create(const GearDir& content, G4LogicalVolume& topVolume,
                  geometry::GeometryTypes type) override;

      /**
       * Creation of the detector geometry from database.
       * @param[in] name      Name of the component in the database.
       * @param[in] topVolume Geant world volume.
       * @param[in] type      Geometry type.
       */
      void createFromDB(const std::string& name, G4LogicalVolume& topVolume,
                        geometry::GeometryTypes type) override;

      /**
       * Creation of payloads.
       * @param content XML data directory.
       * @param iov     Interval of validity.
       */
      void createPayloads(const GearDir& content,
                          const IntervalOfValidity& iov) override;

    private:

      /** BKLM geometry creator. */
      bklm::GeoBKLMCreator m_GeoBKLMCreator;

      /** EKLM geometry creator. */
      EKLM::GeoEKLMCreator m_GeoEKLMCreator;

    };

  }

}
