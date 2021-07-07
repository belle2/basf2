/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>

/* C++ headers. */
#include <cstdint>
#include <string>

namespace Belle2 {

  /**
   * BKLM element numbers.
   */
  class BKLMElementNumbers {

  public:

    /**
     * Constants for section numbers.
     */
    enum Section {

      /** Backward. */
      c_BackwardSection = 0,

      /** Forward. */
      c_ForwardSection = 1,

    };

    /**
     * Constants for sector numbers.
     */
    enum Sector {

      /**
       * Chimney sector:
       * BB3 in 1-based notation;
       * BB2 in 0-based notation.
       */
      c_ChimneySector = 3,

    };

    /**
     * Constants for layer numbers.
     */
    enum Layer {

      /** First RPC layer. */
      c_FirstRPCLayer = 3,

    };

    /**
     * Constants for plane numbers.
     */
    enum Plane {

      /** Z. */
      c_ZPlane = 0,

      /** Phi. */
      c_PhiPlane = 1,

    };

    /**
     * Constructor.
     */
    BKLMElementNumbers();

    /**
     * Destructor.
     */
    ~BKLMElementNumbers();

    /**
     * Get channel number.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    static KLMChannelNumber channelNumber(int section, int sector, int layer,
                                          int plane, int strip);

    /**
     * Get element numbers by channel number.
     * @param[in]  channel Channel number.
     * @param[out] section Section (0-based).
     * @param[out] sector  Sector (1-based).
     * @param[out] layer   Layer (1-based).
     * @param[out] plane   Plane (0-based).
     * @param[out] strip   Strip (1-based).
     */
    static void channelNumberToElementNumbers(
      KLMChannelNumber channel, int* section, int* sector, int* layer, int* plane,
      int* strip);

    /**
     * Get plane number.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     */
    static KLMPlaneNumber planeNumber(int section, int sector, int layer,
                                      int plane);

    /**
     * Get element numbers by plane number.
     * @param[in]  planeGlobal Plane number.
     * @param[out] section     Section (0-based).
     * @param[out] sector      Sector (1-based).
     * @param[out] layer       Layer (1-based).
     * @param[out] plane       Plane (0-based).
     */
    static void planeNumberToElementNumbers(
      KLMPlaneNumber planeGlobal, int* section, int* sector, int* layer,
      int* plane);

    /**
     * Get module number.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static KLMModuleNumber moduleNumber(int section, int sector, int layer,
                                        bool fatalError = true);

    /**
     * Get element numbers by module number.
     * @param[in]  module  Module.
     * @param[out] section Section (0-based).
     * @param[out] sector  Sector (1-based).
     * @param[out] layer   Layer (1-based).
     */
    static void moduleNumberToElementNumbers(
      KLMModuleNumber module, int* section, int* sector, int* layer);

    /**
     * Get sector number.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     */
    static KLMSectorNumber sectorNumber(int section, int sector);

    /**
     * Get layer global number.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     */
    static int layerGlobalNumber(int section, int sector, int layer);

    /**
     * Get number of strips.
     * @param[in] section Section (0-based).
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     */
    static int getNStrips(int section, int sector, int layer, int plane);

    /**
     * Check if section number is correct.
     * @param[in] section    Section (0-based).
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static bool checkSection(int section, bool fatalError = true);

    /**
     * Check if sector number is correct.
     * @param[in] sector     Sector (1-based)
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static bool checkSector(int sector, bool fatalError = true);

    /**
     * Check if layer number is correct.
     * @param[in] layer      Layer (1-based)
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static bool checkLayer(int layer, bool fatalError = true);

    /**
     * Check if plane number is correct.
     * @param[in] plane      Plane (0-based)
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static bool checkPlane(int plane, bool fatalError = true);

    /**
     * Check channel number.
     * @param[in] section    Section (0-based).
     * @param[in] sector     Sector (1-based).
     * @param[in] layer      Layer (1-based).
     * @param[in] plane      Plane (0-based).
     * @param[in] strip      Strip (1-based).
     * @param[in] fatalError Issue fatal error (default) or not.
     */
    static bool checkChannelNumber(
      int section, int sector, int layer, int plane, int strip, bool fatalError = true);

    /**
     * Get HSLB name.
     * @param[in] copper Copper.
     * @param[in] slot   Slot.
     */
    static std::string getHSLBName(int copper, int slot);

    /**
     * Get maximal section number (0-based).
     */
    static constexpr int getMaximalSectionNumber()
    {
      return m_MaximalSectionNumber;
    }

    /**
     * Get maximal sector number (1-based).
     */
    static constexpr int getMaximalSectorNumber()
    {
      return m_MaximalSectorNumber;
    }

    /**
     * Get maximal layer number (1-based).
     */
    static constexpr int getMaximalLayerNumber()
    {
      return m_MaximalLayerNumber;
    }

    /**
     * Get maximal plane number (0-based).
     */
    static constexpr int getMaximalPlaneNumber()
    {
      return m_MaximalPlaneNumber;
    }

    /**
     * Get maximal sector global number.
     */
    static constexpr int getMaximalSectorGlobalNumber()
    {
      return (m_MaximalSectionNumber + 1) * m_MaximalSectorNumber;
    }

    /**
     * Get maximal layer global number.
     */
    static constexpr int getMaximalLayerGlobalNumber()
    {
      return (m_MaximalSectionNumber + 1) * m_MaximalSectorNumber * m_MaximalLayerNumber;
    }

    /**
     * Get total number of channels.
     */
    static constexpr int getTotalChannelNumber()
    {
      return m_TotalChannelNumber;
    }

    /**
     * Get element numbers by layer global number (0-based).
     * @param[in]  layerGlobal  Layer global number.
     * @param[out] section      Section (0-based).
     * @param[out] sector       Sector (1-based).
     * @param[out] layer        Layer (1-based).
     */
    static void layerGlobalNumberToElementNumbers(int layerGlobal, int* section, int* sector, int* layer);

    /**
     * Get section number by module identifier.
     */
    static int getSectionByModule(int module)
    {
      return (module & BKLM_SECTION_MASK) >> BKLM_SECTION_BIT;
    }

    /**
     * Get sector number by module identifier.
     */
    static int getSectorByModule(int module)
    {
      return ((module & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1;
    }

    /**
     * Get layer number by module identifier.
     */
    static int getLayerByModule(int module)
    {
      return ((module & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1;
    }

    /**
     * Get plane number (0 = z, 1 = phi) by module identifier.
     */
    static int getPlaneByModule(int module)
    {
      return (module & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;
    }

    /**
     * Get strip number by module identifier.
     */
    static int getStripByModule(int module)
    {
      return ((module & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1;
    }

    /**
     * Set section number in module identifier.
     * @param[in,out] module  Module identifier.
     * @param[in]     section Section.
     */
    static void setSectionInModule(int& module, int section)
    {
      module = (module & (~BKLM_SECTION_MASK)) | (section << BKLM_SECTION_BIT);
    }

    /**
     * Set sector number in module identifier.
     * @param[in,out] module Module identifier.
     * @param[in]     sector Sector.
     */
    static void setSectorInModule(int& module, int sector)
    {
      module = (module & (~BKLM_SECTOR_MASK)) | ((sector - 1) << BKLM_SECTOR_BIT);
    }

    /**
     * Set layer number in module identifier.
     * @param[in,out] module Module identifier.
     * @param[in]     layer  Layer identifier.
     */
    static void setLayerInModule(int& module, int layer)
    {
      module = (module & (~BKLM_LAYER_MASK)) | ((layer - 1) << BKLM_LAYER_BIT);
    }

    /**
     * Set plane number in module identifier.
     * @param[in,out] module Module identifier.
     * @param[in]     plane  Plane.
     */
    static void setPlaneInModule(int& module, int plane)
    {
      module = (module & (~BKLM_PLANE_MASK)) | (plane << BKLM_PLANE_BIT);
    }

    /**
     * Set strip number in module identifier.
     * @param[in,out] module Module identifier.
     * @param[in]     strip  Strip.
     */
    static void setStripInModule(int& module, int strip)
    {
      module = (module & (~BKLM_STRIP_MASK)) | ((strip - 1) << BKLM_STRIP_BIT);
    }

    /**
     * Get module number by module identifier (the input identifier may contain
     * other data).
     */
    static uint16_t getModuleByModule(int module)
    {
      return module & BKLM_MODULEID_MASK;
    }

    /**
     * Get channel number by module identifier.
     */
    static uint16_t getChannelByModule(int module)
    {
      return module & BKLM_MODULESTRIPID_MASK;
    }

    /**
     * Check whether the hits are from the same module.
     */
    static bool hitsFromSameModule(int module1, int module2)
    {
      return getModuleByModule(module1) == getModuleByModule(module2);
    }

    /**
     * Check whether the hits are from the same plane.
     */
    static bool hitsFromSamePlane(int module1, int module2)
    {
      const int mask = BKLM_MODULEID_MASK | BKLM_PLANE_MASK;
      return (module1 & mask) == (module2 & mask);
    }

    /**
     * Check whether the hits are from the same channel.
     */
    static bool hitsFromSameChannel(int module1, int module2)
    {
      return getChannelByModule(module1) == getChannelByModule(module2);
    }

  protected:

    /** Maximal section number (0-based). */
    static constexpr int m_MaximalSectionNumber = 1;

    /** Maximal sector number (1-based). */
    static constexpr int m_MaximalSectorNumber = 8;

    /** Maximal layer number (1-based). */
    static constexpr int m_MaximalLayerNumber = 15;

    /** Maximal plane number (0-based). */
    static constexpr int m_MaximalPlaneNumber = 1;

    /** Total number of channels (1-based). */
    static constexpr int m_TotalChannelNumber = 21978;

    /** Bit position for strip-1 [0..47]. */
    static constexpr int BKLM_STRIP_BIT = 0;

    /**
     * Bit position for plane-1 [0..1].
     * 0 is inner-plane and phiReadout plane.
     */
    static constexpr int BKLM_PLANE_BIT = 6;

    /** Bit position for layer-1 [0..14]; 0 is innermost. */
    static constexpr int BKLM_LAYER_BIT = 7;

    /**
     * Bit position for sector-1 [0..7].
     * 0 is on the +x axis and 2 is on the +y axis
     */
    static constexpr int BKLM_SECTOR_BIT = 11;

    /** Bit position for detector end [0..1]; forward is 0. */
    static constexpr int BKLM_SECTION_BIT = 14;

    /** Bit mask for strip-1 [0..47]. */
    static constexpr int BKLM_STRIP_MASK = (63 << BKLM_STRIP_BIT);

    /** Bit mask for plane-1 [0..1]; 0 is inner-plane and phiReadout plane, */
    static constexpr int BKLM_PLANE_MASK = (1 << BKLM_PLANE_BIT);

    /** Bit mask for layer-1 [0..15]; 0 is innermost and 14 is outermost. */
    static constexpr int BKLM_LAYER_MASK = (15 << BKLM_LAYER_BIT);

    /**
     * Bit mask for sector-1 [0..7].
     * 0 is on the +x axis and 2 is on the +y axis.
     */
    static constexpr int BKLM_SECTOR_MASK = (7 << BKLM_SECTOR_BIT);

    /** Bit mask for detector end [0..1]; forward is 0. */
    static constexpr int BKLM_SECTION_MASK = (1 << BKLM_SECTION_BIT);

    /** Bit mask for module identifier. */
    static constexpr int BKLM_MODULEID_MASK =
      BKLM_SECTION_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK;

    /** Bit mask for module-and-strip identifier. */
    static constexpr int BKLM_MODULESTRIPID_MASK =
      BKLM_SECTION_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK | BKLM_PLANE_MASK |
      BKLM_STRIP_MASK;

  };

}
