
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store the fill pattern of colliding bunches.
   *
   * Default: if setBucket is not called, works as every second bucket is filled (even bucket numbers).
   */
  class BunchStructure: public TObject {

  public:

    /**
     * Default constructor
     */
    BunchStructure()
    {}

    /**
     * Sets the fill status of RF bucket to true
     * @param i bucket number (modulo c_RFBuckets applied internally)
     */
    void setBucket(unsigned i);

    /**
     * Returns the number of RF buckets per beam revolution
     * @return the number of RF buckets per beam revolution
     */
    unsigned getRFBucketsPerRevolution() const {return c_RFBuckets;}

    /**
     * Returns fill pattern
     * @return fill pattern
     */
    const std::vector<bool>& getFillPattern() const {return m_fillPattern;}

    /**
     * Returns the fill status of RF bucket
     * @param i bucket number (modulo c_RFBuckets applied internally)
     * @return true if bucket is filled
     */
    bool getBucket(unsigned i) const;

    /**
     * Checks if the fill pattern is really set
     * @return true, if the fill pattern is set
     */
    bool isSet() const {return (not m_fillPattern.empty());}

    /**
     * Returns the number of filled buckets
     * @return the number of filled buckets
     */
    unsigned getNumOfFilledBuckets() const;

    /**
     * Generates RF bucket number according to the fill pattern
     * @return bucket number
     */
    unsigned generateBucketNumber() const;


  private:

    /**
     * Number of RF buckets per beam revolution
     */
    enum {c_RFBuckets = 5120};

    std::vector<bool> m_fillPattern; /**< fill pattern, true means the bucket is filled */

    /** cache for the filled bucket numbers */
    mutable std::vector<unsigned> m_filledBuckets; //! dont't write out

    ClassDef(BunchStructure, 1); /**< ClassDef */

  };

} //Belle2 namespace

