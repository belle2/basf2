/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_RECOTIMEBASE_H
#define SVD_RECOTIMEBASE_H

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <sstream>
#include <string>
#include <algorithm>

namespace Belle2 {

  /**
   * The SVD RecoTimeBase class.
   *
   * The SVDRecoTimeBase holds data on time binning used by the signal fitter.
   * NB:
   * For future use, the object contains VxdID and side inormation.
   */

  class SVDRecoTimeBase : public RelationsObject {

  public:

    /** Types for bins array.
     */
    typedef float BinnedDataType;
    /** BinEdgesArray[i] is the left edge of bin i */
    typedef std::vector<BinnedDataType> BinEdgesArray;

    /** Constructor using a stl container of time bin probabilities.
     * @param sensorID Sensor VXD ID.
     * @param isU True if u strip, false if v.
     * @param bins Array defining binning used by the time fitter.
     */
    template<typename T>
    SVDRecoTimeBase(VxdID sensorID, bool isU, const T& bins):
      m_sensorID(sensorID), m_isU(isU)
    {
      std::copy(bins.begin(), bins.end(), std::back_inserter(m_bins));
    }

    /** Default constructor for the ROOT IO. */
    SVDRecoTimeBase() : SVDRecoTimeBase(0, true, std::vector<double>( {0.0}))
    { }

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get raw sensor ID.
     * For use in Python
     * @return basetype ID of the sensor.
     */
    VxdID::baseType getRawSensorID() const { return m_sensorID; }

    /** Get strip direction.
     * @return true if u, false if v.
     */
    bool isUStrip() const { return m_isU; }

    /** Get number of bins.
     * @return number of bins used by the time fitter.
     */
    std::size_t getNBins() const { return m_bins.size() - 1; }

    /** Get the bins array.
     * @return array of bins such that i-th bin has edges at bins[i] and bins[i+1].
     */
    const BinEdgesArray& getBins() const
    {
      return m_bins;
    }

    /** Display main parameters in this object */
    std::string toString() const
    {
      VxdID thisSensorID(m_sensorID);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID)
         << " side: " << ((m_isU) ? "U" : "V") << " bins: ";
      std::copy(m_bins.begin(), m_bins.end(),
                std::ostream_iterator<BinnedDataType>(os, " "));
      return os.str();
    }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU; /**< True if U, false if V. */
    BinEdgesArray m_bins; /**< Bins used by the time fitter. */

    ClassDef(SVDRecoTimeBase, 2) /**< needed by root*/

  }; // class SVDRecoTimeBase

} // end namespace Belle2

#endif // SVD_RECOTIMEBASE_H
