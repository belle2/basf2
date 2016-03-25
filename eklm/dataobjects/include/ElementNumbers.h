/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_ELEMENTNUMBERS_H
#define EKLM_ELEMENTNUMBERS_H

namespace Belle2 {

  namespace EKLM {

    /**
     * EKLM element numbers. This class is not a data object but is used by
     * data objects.
     */
    class ElementNumbers {

    public:

      /**
       * Constructor.
       */
      ElementNumbers();

      /**
       * Destructor.
       */
      virtual ~ElementNumbers();

      /**
       * Check if endcap number is correct (fatal error if not).
       * @param[in] endcap Endcap number.
       */
      void checkEndcap(int endcap) const;

      /**
       * Check if layer number is correct (fatal error if not).
       * @param[in] layer Layer number.
       */
      void checkLayer(int layer) const;

      /**
       * Check if detector layer number is correct (fatal error if not).
       * Endcap number must be checked separately.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       */
      void checkDetectorLayer(int endcap, int layer) const;

      /**
       * Check if sector number is correct (fatal error if not).
       * @param[in] sector Sector number.
       */
      void checkSector(int sector) const;

      /**
       * Check if plane number is correct (fatal error if not).
       * @param[in] plane Plane number.
       */
      void checkPlane(int plane) const;

      /**
       * Check if segment number is correct (fatal error if not).
       * @param[in] segment Segment number.
       */
      void checkSegment(int segment) const;

      /**
       * Check if strip number is correct (fatal error if not).
       * @param[in] strip Strip number.
       */
      void checkStrip(int strip) const;

      /**
       * Get detector layer number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @return Number of layer.
       * @details
       * Number is from 1 to 12 + 14 = 26.
       */
      int detectorLayerNumber(int endcap, int layer) const;

      /**
       * Get sector number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @return Number of sector.
       * @details
       * Number is from 1 to 4*14*2 = 112.
       */
      int sectorNumber(int endcap, int layer, int sector) const;

      /**
       * Get plane number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @return Number of plane.
       * @details
       * Number is from 1 to 2*4*26 = 208.
       */
      int planeNumber(int endcap, int layer, int sector, int plane) const;

      /**
       * Get segment number.
       * @param[in] endcap  Endcap number.
       * @param[in] layer   Layer number.
       * @param[in] sector  Sector number.
       * @param[in] plane   Plane number.
       * @param[in] segment Segment number.
       * @return Number of strip.
       * @details
       * Number is from 1 to 5*2*4*26 = 1040.
       */
      int segmentNumber(int endcap, int layer, int sector, int plane,
                        int strip) const;

      /**
       * Get strip number.
       * @param[in] endcap Endcap number.
       * @param[in] layer  Layer number.
       * @param[in] sector Sector number.
       * @param[in] plane  Plane number.
       * @param[in] strip  Strip number.
       * @return Number of strip.
       * @details
       * Number is from 1 to 75*2*4*26 = 15600.
       */
      int stripNumber(int endcap, int layer, int sector, int plane,
                      int strip) const;

      /**
       * Strip local number (in a plane).
       * @param[in] strip Global strip number.
       */
      int stripLocalNumber(int strip) const;

      /**
       * Get maximal strip number.
       */
      int getMaximalStripNumber() const;

    protected:

      /** Maximal endcap number. */
      const int m_MaximalEndcapNumber;

      /** Maximal layer number. */
      const int m_MaximalLayerNumber;

      /** Maximal detector layer number. */
      const int m_MaximalDetectorLayerNumber[2];

      /** Maximal sector number. */
      const int m_MaximalSectorNumber;

      /** Maximal plane number. */
      const int m_MaximalPlaneNumber;

      /** Maximal segment number. */
      const int m_MaximalSegmentNumber;

      /** Maximal strip number. */
      const int m_MaximalStripNumber;

    };

  }

}

#endif

