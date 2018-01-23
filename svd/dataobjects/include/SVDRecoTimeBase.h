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
#include <svd/dataobjects/SVDModeByte.h>
#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <numeric>
#include <functional>

namespace Belle2 {

  /**
   * The SVD RecoTimeBase class.
   *
   * The SVDRecoTimeBase holds data on time binning used by the signal fitter.
   * NB:
   * For future use, the object contains VxdID, side and mode information.
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
     * @param m_binEdges Array defining binning used by the time fitter.
     * @param mode SVDModeByte structure, packed trigger time bin and DAQ
     * mode.
     */
    template<typename T>
    SVDRecoTimeBase(VxdID sensorID, bool isU, const T& bins,
                    SVDModeByte mode = SVDModeByte()):
      m_sensorID(sensorID), m_isU(isU), m_mode(mode.getID())
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

    /** Get the SVDMOdeByte object containing information on trigger FADCTime and DAQ mode.
     * @return the SVDModeByte object of the digit
     */
    SVDModeByte getModeByte() const
    { return m_mode; }

    /** Display main parameters in this object */
    std::string toString() const
    {
      VxdID thisSensorID(m_sensorID);
      SVDModeByte thisMode(m_mode);

      std::ostringstream os;
      os << "VXDID : " << m_sensorID << " = " << std::string(thisSensorID)
         << " side: " << ((m_isU) ? "U" : "V") << " bins: ";
      std::copy(m_bins.begin(), m_bins.end(),
                std::ostream_iterator<BinnedDataType>(os, " "));
      os << " mode: " << thisMode << std::endl;
      return os.str();
    }

  private:

    VxdID::baseType m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU; /**< True if U, false if V. */
    BinEdgesArray m_bins; /** Bins used by the time fitter. */
    SVDModeByte::baseType m_mode; /**< Mode byte, trigger FADCTime + DAQ mode */

    ClassDef(SVDRecoTimeBase, 1)

  }; // class SVDRecoTimeBase

} // end namespace Belle2

#endif // SVD_RECOTIMEBASE_H
