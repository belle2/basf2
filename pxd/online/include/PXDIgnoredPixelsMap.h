/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>

#include <functional>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace Belle2 {
  /** This class provides a check for ignored (=cold, hot or otherwise deffective)
   * pixels for the use in PXD data reconstruction.
   * The class has non-intrusive behavior, that is, it will not interfere when
   * queries on non-existent sensors are asked.
   * Treatment of single masked pixels and masked ranges (rows, cols, ...) is
   * different for performance reasons.
   *
   * NOTE: More optimization is possible:
   *  - single rows and cols can go to hash tables (like pixels), as they can be uniquely identified.
   *    Then we would have m_MaskRows, m_MaskCols etc.
   *  - lambdas for ranges of cols/rows can be simplified (2 less comparisons)
   * or even
   *  - instead hash tables, one could use directly vectors/arrays/std::bitset and only pixels, but
   *    with much more memory for map of all sensors (at least 8Mbit for whole Belle2 PXD & std::bitset)
   */
  class PXDIgnoredPixelsMap {
  public:
    /** Simple structure for a pixel, u = map_pixel.first, v = map_pixel.second */
    typedef std::pair<unsigned short, unsigned short> map_pixel;
    /** Prototype for lambda function used to check if a pixel is in masked range */
    typedef std::function< bool(unsigned short, unsigned short) > pixel_range_test_prototype;
    /** Structure to hold set of masked pixel ranges ordered from largest to smallest by their area (unsigned int) */
    typedef std::multimap< unsigned int, pixel_range_test_prototype, std::greater<unsigned int> > IgnoredPixelsRangeSet;
    /** Structure to hold set of masked single pixels indexed by their unique id (unsigned int), stored in hash table */
    typedef std::unordered_set< unsigned int> IgnoredSinglePixelsSet;

    /** Constructor
     * @param xml_filename is the name of the xml file containing the map.
     */
    explicit PXDIgnoredPixelsMap(const std::string& xml_filename);

    /** No default constructor */
    PXDIgnoredPixelsMap() = delete;

    /** Get the set of ignored pixels for a sensor.
     *
     * @param id VxdID of the required sensor
     * @return the set of ignored pixels, and empty set for non-existent VxdID.
     */
    const std::set<map_pixel> getIgnoredPixels(VxdID id);

    /** Check whether a pixel on a given sensor is OK or not.
     * @param id VxdID of the sensor
     * @param pixel Pixel constructed e.g. as map_pixel(u, v)
     * @return true if pixel or the id is not found in the list, otherwise false.
     */
    bool pixelOK(VxdID id, map_pixel pixel);

  private:

    /** Structure holding sets of ignored pixel ranges for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, IgnoredPixelsRangeSet> m_Map;
    /** Structure holding sets of ingored single pixels for all sensors by sensor id (unsigned short). */
    std::unordered_map<unsigned short, IgnoredSinglePixelsSet> m_MapSingles;

    /** Set of ignored pixel ranges for the most currently queried sensor */
    IgnoredPixelsRangeSet m_lastIgnored;
    /** Set of ignored single pixels for the most currently queried sensor */
    IgnoredSinglePixelsSet m_lastIgnoredSingles;

    /** The most currently queried sensor number */
    VxdID m_lastSensorID;
    /** The most currently queried sensor number of V pixels (for quick pixel uid evaluation) */
    unsigned short m_lastSensorVCells;

  };

} // namespace Belle2

