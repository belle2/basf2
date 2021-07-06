/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <pxd/dbobjects/PXDMaskedPixelPar.h>
#include <pxd/dbobjects/PXDDeadPixelPar.h>
#include <framework/database/DBObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <memory>

namespace Belle2 {

  namespace PXD {
    /**
    * Singleton class for managing pixel masking for the PXD.
    */
    class PXDPixelMasker {

    public:

      /** Initialize the PXDPixelMasker */
      void initialize();

      /** Set masked pixels from DB. */
      void setMaskedPixels();

      /** Set dead pixels from DB. */
      void setDeadPixels();

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
       * @param uid uCell of single pixel
       * @param vid vCell of single pixel
       * @return true if pixel is OK, otherwise false.
       */
      bool pixelOK(VxdID id, unsigned int uid, unsigned int vid) const;

      /** Check whether a pixel on a given sensor is dead or not.
       * @param id VxdID of the sensor
       * @param uid uCell of single pixel
       * @param vid vCell of single pixel
       * @return true if pixel DEAD, otherwise false.
       */
      bool pixelDead(VxdID id, unsigned int uid, unsigned int vid) const;

      /** Return masked pixel payload */
      const PXDMaskedPixelPar& getMaskedPixelParameters() const {return m_maskedPixels;}

      /** Return dead pixel payload */
      const PXDDeadPixelPar& getDeadPixelParameters() const {return m_deadPixels;}

    private:

      /** Singleton class, hidden constructor */
      PXDPixelMasker() {};
      /** Singleton class, forbidden copy constructor */
      PXDPixelMasker(const PXDPixelMasker&) = delete;
      /** Singleton class, forbidden assignment operator */
      PXDPixelMasker& operator=(const PXDPixelMasker&) = delete;

      /** Masked pixels retrieved from DB. */
      std::unique_ptr<DBObjPtr<PXDMaskedPixelPar>> m_maskedPixelsFromDB;

      /** Dead pixels retrieved from DB. */
      std::unique_ptr<DBObjPtr<PXDDeadPixelPar>> m_deadPixelsFromDB;

      /** List of masked pixels. */
      PXDMaskedPixelPar m_maskedPixels;

      /** List of masked pixels. */
      PXDDeadPixelPar m_deadPixels;
    };
  }
} //Belle2 namespace
