/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Benjamin Schwenker                                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <pxd/dbobjects/PXDGainMapPar.h>
#include <framework/database/DBObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <memory>

namespace Belle2 {

  namespace PXD {
    /**
    * Singleton class for managing gain corrections for the PXD.
    */
    class PXDGainCalibrator {

    public:

      /** Initialize the PXDGainCalibrator */
      void initialize();

      /** Set gains from DB. */
      void setGains();

      /** Main (and only) way to access the PXDGainCalibrator. */
      static PXDGainCalibrator& getInstance();

      /** Get gain correction
       * @param sensorID unique ID of the sensor
       * @param uid uCell of single pixel
       * @param vid vCell of single pixel
       * @return gain correction
       */
      float getGainCorrection(VxdID id, unsigned int uid, unsigned int vid) const;

      /** Return current gain correction payload */
      const PXDGainMapPar& getGainMapParameters() const {return m_gains;}

    private:

      /** Singleton class, hidden constructor */
      PXDGainCalibrator() {};
      /** Singleton class, forbidden copy constructor */
      PXDGainCalibrator(const PXDGainCalibrator&) = delete;
      /** Singleton class, forbidden assignment operator */
      PXDGainCalibrator& operator=(const PXDGainCalibrator&) = delete;

      /** Masked pixels retrieved from DB. */
      std::unique_ptr<DBObjPtr<PXDGainMapPar>> m_gainsFromDB;

      /** Map of gain corrections. */
      PXDGainMapPar m_gains;
    };
  }
} //Belle2 namespace
