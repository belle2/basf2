/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD_STRIP_NOISE_MAP_H_
#define SVD_STRIP_NOISE_MAP_H_

#include <vxd/dataobjects/VxdID.h>
#include <svd/online/SVDPar.h>
#include <svd/online/SVDOnlineToOfflineMap.h>

#include <string>

namespace Belle2 {
  /* This class provides a list of pedestal, noise and
   * threshold values in SVD strips.
   * This class uses arrays (not std::map) for the list
   * in order to make accessing speed faster.
   */

  /** strip noise map*/
  class SVDStripNoiseMap {
  public:

    /** Constructor */
    SVDStripNoiseMap(SVDOnlineToOfflineMap* onl2offl_map_ptr = nullptr);
    /** Constructor including map initialization */
    SVDStripNoiseMap(SVDOnlineToOfflineMap* onl2offl_map_ptr,
                     const std::string& noisefilename);

    /** Destructor */
    ~SVDStripNoiseMap() {};

    /** Initialize maps with input noisefile */
    int initializeMap(const std::string& noisefilename = "");

    /** Get pedestal, noise, and threshold values.
     * @param id VxdID of the required sensor, with segment number 0 for v, 1 for u
     * @param is_u true: u-side (p-side), false: v-side (n-side)
     * @param strip Strip number
     * @return pedestal value
     */
    float getPedestal(VxdID id, bool is_u, short strip);

    /** Get pedestal, noise, and threshold values.
     * @param id VxdID of the required sensor, with segment number 0 for v, 1 for u
     * @param is_u true: u-side (p-side), false: v-side (n-side)
     * @param strip Strip number
     * @return noise value
     */
    float getNoise(VxdID id, bool is_u, short strip);

    /** Get pedestal, noise, and threshold values.
     * @param id VxdID of the required sensor, with segment number 0 for v, 1 for u
     * @param is_u true: u-side (p-side), false: v-side (n-side)
     * @param strip Strip number
     * @return threshold value
     */
    float getThreshold(VxdID id, bool is_u, short strip);

    /** Check whether the strip is available or not.
     * @param id VxdID of the required sensor, with segment number 0 for v, 1 for u
     * @param is_u true: u-side (p-side), false: v-side (n-side)
     * @param strip Strip number
     * @return true if the strip is available, otherwise false
     */
    bool  isGood(VxdID id, bool is_u, short strip);

  private:

    /** Pointer to SVDOnlineToOfflineMap.
     * This map represents relationship between
     * ADC+APV and Layer+Ladder+Sensor and it is
     * necessary to convert the noise file to
     * pedestal, noise, and threshold maps with
     * initializeMap() function.
     */
    SVDOnlineToOfflineMap* m_onl2offl_map_ptr;

    /** Pedestal map for all strips in ladders.
     */
    float m_pedestalMap [SVDPar::nSensorID][SVDPar::maxStrip];

    /** noise map for all strips in ladders.
     */
    float m_noiseMap    [SVDPar::nSensorID][SVDPar::maxStrip];

    /** threshold map for all strips in ladders.
     */
    float m_thresholdMap[SVDPar::nSensorID][SVDPar::maxStrip];

    /** Good strip map
     */
    bool  m_goodStripMap[SVDPar::nSensorID][SVDPar::maxStrip];

  };

} // namespace Belle2

#endif
