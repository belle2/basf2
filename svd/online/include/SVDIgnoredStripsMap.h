/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_IGNORED_STRIPS_MAP_H_
#define SVD_IGNORED_STRIPS_MAP_H_

#include <vxd/dataobjects/VxdID.h>
#include <unordered_map>
#include <set>


namespace Belle2 {
  /** This class provides a list of ignored (=cold, hot or otherwise deffective)
   * strips for the use in SVD data reconstruction.
   * The class has non-intrusive behavior, that is, it will not interfere when
   * queries on non-existent sensors are asked.
   */

  class SVDIgnoredStripsMap {
  public:

    /** A set of ignored strips. */
    typedef std::set<unsigned short> IgnoredStripsSet;

    /** Constructor
     * @param xml_filename is the name of the xml file containing the map.
     */
    explicit SVDIgnoredStripsMap(const std::string& xml_filename);

    /** No default constructor */
    SVDIgnoredStripsMap() = delete;

    /** Get the set of ignored strips for a sensor.
     * Use to save map searches.
     * @param id VxdID of the required sensor, with segment number 0 for v, 1 for u
     * @return the set of ignored strips, and empty set for non-existent VxdID.
     */
    const std::set<unsigned short>& getIgnoredStrips(VxdID id);

    /** Check whether a strip on a given sensor is OK or not.
     * @param id VxdID of the sensor, with segment number 0 for v, 1 for u
     * @param strip Strip number
     * @return true if strip or the id is not found in the list, otherwise false.
     */
    bool stripOK(VxdID id, unsigned short strip);

  private:

    /** Structure holiding sets of ingored strips for all sensors. */
    std::unordered_map<unsigned short, IgnoredStripsSet> m_Map;

    /** Set of ingored strips for the most currently queried sensor */
    IgnoredStripsSet m_lastIgnored;

    /** The most currently queried sensor number */
    VxdID m_lastSensorID;

  };

} // namespace Belle2
#endif

