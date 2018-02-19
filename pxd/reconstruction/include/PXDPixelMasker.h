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
#include <pxd/dbobjects/PXDMaskedPixelPar.h>
#include <framework/database/DBObjPtr.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  namespace PXD {
    /**
    * Singleton class for managing pixel masking for the PXD.
    */
    class PXDPixelMasker {

    public:

      /** Read cluster position corrections from DataBase */
      void initialize()
      {
        DBObjPtr<PXDMaskedPixelPar> dbObj;
        if (dbObj) {
          // Store payload
          m_maskedPixelPar = *dbObj;
          // Remember that payloads are initialized now
          m_isInitialized = true;
        } else {
          // Check that we found the objects and if not report the problem
          // Keep low profile because there is always a fallback
          B2WARNING("Cannot initialize PXDPixelMasker: The payload PXDMaskedPixelPar is missing.");
        }
      }

      /** Get initialization status. */
      bool isInitialized() const { return m_isInitialized; }

      /** Main (and only) way to access the PXDPixelMasker. */
      static PXDPixelMasker& getInstance();

      /** Mask single pixel
       *
       * @param id VxdID of the required sensor
       * @param uid uCell of single pixel to mask
       * @param vid vCell of single pixel to mask
       */
      void maskSinglePixel(VxdID id, unsigned int uid, unsigned int vid);


      /** Check whether a pixel on a given sensor is OK or not.
       * @param id VxdID of the sensor
       * @param uid uCell of single pixel to mask
       * @param vid vCell of single pixel to mask
       * @return true if pixel or the id is not found in the list, otherwise false.
       */
      bool pixelOK(VxdID id, unsigned int uid, unsigned int vid) const;

      /** Return masked pixel payload */
      const PXDMaskedPixelPar& getMaskedPixelParameters() const {return m_maskedPixelPar;}

    private:

      /** Singleton class, hidden constructor */
      PXDPixelMasker() {};
      /** Singleton class, forbidden copy constructor */
      PXDPixelMasker(const PXDPixelMasker&) = delete;
      /** Singleton class, forbidden assignment operator */
      PXDPixelMasker& operator=(const PXDPixelMasker&) = delete;

      /** Flag to indicate if Database payloads found. Set to true by a successful call to initialize(), otherwise false. */
      bool m_isInitialized = false;
      /** Masked single pixels in PXD*/
      PXDMaskedPixelPar m_maskedPixelPar;

    };
  }
} //Belle2 namespace
