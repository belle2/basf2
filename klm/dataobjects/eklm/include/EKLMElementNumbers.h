/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM element numbers.
   * All class members are constants, they are not stored.
   */
  class EKLMElementNumbers : public TObject {

  public:

    /**
     * Constants for section numbers.
     */
    enum Section {

      /** Backward. */
      c_BackwardSection = 1,

      /** Forward. */
      c_ForwardSection = 2,

    };

    /**
     * Instantiation.
     */
    static const EKLMElementNumbers& Instance();

    /**
     * Check if section number is correct.
     * @param[in] section    Section number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the section number is correct.
     */
    bool checkSection(int section, bool fatalError = true) const;

    /**
     * Check if layer number is correct.
     * @param[in] layer      Layer number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the layer number is correct.
     */
    bool checkLayer(int layer, bool fatalError = true) const;

    /**
     * Check if detector layer number is correct (fatal error if not).
     * Section number must be checked separately.
     * @param[in] section    Section number.
     * @param[in] layer      Layer number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the detector layer number is correct.
     */
    bool checkDetectorLayer(int section, int layer,
                            bool fatalError = true) const;

    /**
     * Check if sector number is correct (fatal error if not).
     * @param[in] sector     Sector number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the sector number is correct.
     */
    bool checkSector(int sector, bool fatalError = true) const;

    /**
     * Check if plane number is correct (fatal error if not).
     * @param[in] plane      Plane number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the plane number is correct.
     */
    bool checkPlane(int plane, bool fatalError = true) const;

    /**
     * Check if segment number is correct (fatal error if not).
     * @param[in] segment    Segment number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the segment number is correct.
     */
    bool checkSegment(int segment, bool fatalError = true) const;

    /**
     * Check if strip number is correct (fatal error if not).
     * @param[in] strip      Strip number.
     * @param[in] fatalError Issue fatal error (default) or not.
     * @return True if the strip number is correct.
     */
    bool checkStrip(int strip, bool fatalError = true) const;

    /**
     * Get detector layer number.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     * @return Number of layer.
     * @details
     * Number is from 1 to 12 + 14 = 26.
     */
    int detectorLayerNumber(int section, int layer) const;

    /**
     * Get element numbers by detector layer global number.
     * @param[in]  layerGlobal Layer global number.
     * @param[out] section     Section number.
     * @param[out] layer       Layer number.
     */
    void layerNumberToElementNumbers(
      int layerGlobal, int* section, int* layer) const;

    /**
     * Get sector number.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     * @param[in] sector Sector number.
     * @return Number of sector.
     * @details
     * Number is from 1 to 4*26 = 104.
     */
    int sectorNumber(int section, int layer, int sector) const;

    /**
     * Get sector number (KLM order of elements: section, sector, layer).
     * @param[in] section Section number.
     * @param[in] sector  Sector number.
     * @return Number of sector.
     * @details
     * Number is from 1 to 2*4 = 8.
     */
    int sectorNumberKLMOrder(int section, int sector) const;

    /**
     * Get element numbers by sector global number.
     * @param[in]  sectorGlobal Sector global number.
     * @param[out] section      Section number.
     * @param[out] layer        Layer number.
     * @param[out] sector       Sector number.
     */
    void sectorNumberToElementNumbers(
      int sectorGlobal, int* section, int* layer, int* sector) const;

    /**
     * Get plane number.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @return Number of plane.
     * @details
     * Number is from 1 to 2*4*26 = 208.
     */
    int planeNumber(int section, int layer, int sector, int plane) const;

    /**
     * Get element numbers by plane global number.
     * @param[in]  planeGlobal Plane global number.
     * @param[out] section     Section number.
     * @param[out] layer       Layer number.
     * @param[out] sector      Sector number.
     * @param[out] plane       Plane number.
     */
    void planeNumberToElementNumbers(
      int planeGlobal, int* section, int* layer, int* sector, int* plane)
    const;

    /**
     * Get segment number.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] segment Segment number.
     * @return Number of strip.
     * @details
     * Number is from 1 to 5*2*4*26 = 1040.
     */
    int segmentNumber(int section, int layer, int sector, int plane,
                      int segment) const;

    /**
     * Get element numbers by segment global number.
     * @param[in]  segmentGlobal Segment global number.
     * @param[out] section       Section number.
     * @param[out] layer         Layer number.
     * @param[out] sector        Sector number.
     * @param[out] plane         Plane number.
     * @param[out] segment       Segment number.
     */
    void segmentNumberToElementNumbers(
      int segmentGlobal, int* section, int* layer, int* sector, int* plane,
      int* segment) const;

    /**
     * Get strip number.
     * @param[in] section Section number.
     * @param[in] layer   Layer number.
     * @param[in] sector  Sector number.
     * @param[in] plane   Plane number.
     * @param[in] strip   Strip number.
     * @return Number of strip.
     * @details
     * Number is from 1 to 75*2*4*26 = 15600.
     */
    int stripNumber(int section, int layer, int sector, int plane,
                    int strip) const;

    /**
     * Get element numbers by strip global number.
     * @param[in]  stripGlobal Strip global number.
     * @param[out] section     Section number.
     * @param[out] layer       Layer number.
     * @param[out] sector      Sector number.
     * @param[out] plane       Plane number.
     * @param[out] strip       Strip number.
     */
    void stripNumberToElementNumbers(
      int stripGlobal, int* section, int* layer, int* sector, int* plane,
      int* strip) const;

    /**
     * Get section number by global strip number.
     * @param[in] stripGlobal Global strip number.
     */
    int getSectionByGlobalStrip(int stripGlobal);

    /**
     * Get layer number by global strip number.
     * @param[in] stripGlobal Global strip number.
     */
    int getLayerByGlobalStrip(int stripGlobal);

    /**
     * Get sector number by global strip number.
     * @param[in] stripGlobal Global strip number.
     */
    int getSectorByGlobalStrip(int stripGlobal);

    /**
     * Get plane number by global strip number.
     * @param[in] stripGlobal Global strip number.
     */
    int getPlaneByGlobalStrip(int stripGlobal);

    /**
     * Get strip number by global strip number.
     * @param[in] stripGlobal Global strip number.
     */
    int getStripByGlobalStrip(int stripGlobal);

    /**
     * Get HSLB name.
     * @param[in] copper Copper.
     * @param[in] slot   Slot.
     */
    static std::string getHSLBName(int copper, int slot);

    /**
     * Get maximal section number.
     */
    static constexpr int getMaximalSectionNumber()
    {
      return m_MaximalSectionNumber;
    }

    /**
     * Get maximal layer number.
     */
    static constexpr int getMaximalLayerNumber()
    {
      return m_MaximalLayerNumber;
    }

    /**
     * Get maximal detector layer number.
     * @param[in] section Section number.
     */
    int getMaximalDetectorLayerNumber(int section) const;

    /**
     * Get maximal sector number.
     */
    static constexpr int getMaximalSectorNumber()
    {
      return m_MaximalSectorNumber;
    }

    /**
     * Get maximal plane number.
     */
    static constexpr int getMaximalPlaneNumber()
    {
      return m_MaximalPlaneNumber;
    }

    /**
     * Get maximal segment number.
     */
    static constexpr int getMaximalSegmentNumber()
    {
      return m_MaximalSegmentNumber;
    }

    /**
     * Get maximal strip number.
     */
    static constexpr int getMaximalStripNumber()
    {
      return m_MaximalStripNumber;
    }

    /**
     * Get maximal detector layer global number.
     */
    static constexpr int getMaximalLayerGlobalNumber()
    {
      return m_MaximalDetectorLayerNumber[0] + m_MaximalDetectorLayerNumber[1];
    }

    /**
     * Get maximal sector global number.
     */
    static constexpr int getMaximalSectorGlobalNumber()
    {
      return m_MaximalSectorNumber * getMaximalLayerGlobalNumber();
    }

    /**
     * Get maximal sector global number with KLM ordering (section, sector).
     */
    static constexpr int getMaximalSectorGlobalNumberKLMOrder()
    {
      return m_MaximalSectionNumber * m_MaximalSectorNumber;
    }

    /**
     * Get maximal plane global number.
     */
    static constexpr int getMaximalPlaneGlobalNumber()
    {
      return m_MaximalPlaneNumber * getMaximalSectorGlobalNumber();
    }

    /**
     * Get maximal segment global number.
     */
    static constexpr int getMaximalSegmentGlobalNumber()
    {
      return m_MaximalSegmentNumber * getMaximalPlaneGlobalNumber();
    }

    /**
     * Get maximal strip global number.
     */
    static constexpr int getMaximalStripGlobalNumber()
    {
      return m_MaximalStripNumber * getMaximalPlaneGlobalNumber();
    }

    /**
     * Get number of strips in a layer.
     */
    static constexpr int getNStripsLayer()
    {
      return getNStripsSector() * m_MaximalSectorNumber;
    }

    /**
     * Get number of strips in a sector.
     */
    static constexpr int getNStripsSector()
    {
      return m_MaximalPlaneNumber * m_MaximalStripNumber;
    }

    /**
     * Get number of strips in a segment.
     */
    static constexpr int getNStripsSegment()
    {
      return m_NStripsSegment;
    }

    /**
     * Get total number of channels.
     */
    static constexpr int getTotalChannelNumber()
    {
      return m_TotalChannelNumber;
    }

  private:

    /**
     * Constructor.
     */
    EKLMElementNumbers();

    /**
     * Destructor.
     */
    ~EKLMElementNumbers();

  protected:

    /** Maximal section number. */
    static constexpr int m_MaximalSectionNumber = 2;

    /** Maximal layer number. */
    static constexpr int m_MaximalLayerNumber = 14;

    /** Maximal detector layer number. */
    static constexpr int m_MaximalDetectorLayerNumber[2] = {12, 14};

    /** Maximal sector number. */
    static constexpr int m_MaximalSectorNumber = 4;

    /** Maximal plane number. */
    static constexpr int m_MaximalPlaneNumber = 2;

    /** Maximal segment number. */
    static constexpr int m_MaximalSegmentNumber = 5;

    /** Maximal strip number. */
    static constexpr int m_MaximalStripNumber = 75;

    /** Number of strips in a segment. */
    static constexpr int m_NStripsSegment = 15;

    /** Total number of channels. */
    static constexpr int m_TotalChannelNumber = 15600;

  private:

    /** Class version. */
    ClassDef(EKLMElementNumbers, 3);

  };

}
