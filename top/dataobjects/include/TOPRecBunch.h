/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TH1F.h>
#include <vector>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to store results of TOPBunchFinder
   */

  class TOPRecBunch : public RelationsObject {
  public:

    /**
     * Reconstructed bucket status
     */
    enum EBucketStatus {
      c_Unknown = -1, /**< not known */
      c_Empty = 0,    /**< bucket is empty */
      c_Filled = 1,   /**< bucket is filled */
    };

    /**
     * Default constructor
     */
    TOPRecBunch()
    {}

    /**
     * Clear reconstruction data members
     */
    void clearReconstructed()
    {
      m_recBunchNo = 0;
      m_recTime = 0;
      m_currentOffset = 0;
      m_averageOffset = 0;
      m_currentOffsetError = 0;
      m_averageOffsetError = 0;
      m_numTracks = 0;
      m_usedTracks = 0;
      m_nodEdx = 0;
      m_fineSearch = false;
      m_histograms.clear();
      m_recValid = false;
      m_minChi2 = 0;
      m_detector = Const::invalidDetector;
      m_bucketNumber = c_Unknown;
      m_isBucketFilled = c_Unknown;
    }

    /**
     * Set reconstructed relative bunch number and time
     * @param bunchNo relative bunch number
     * @param time relative bunch time
     * @param currentOffset current offset
     * @param currentOffsetError uncertainty of current offset
     * @param averageOffset average offset
     * @param averageOffsetError uncertainty of average offset
     * @param detector a component providing the time seed
     */
    void setReconstructed(int bunchNo, double time,
                          double currentOffset, double currentOffsetError,
                          double averageOffset, double averageOffsetError,
                          Const::EDetector detector)
    {
      m_recBunchNo = bunchNo;
      m_recTime = time;
      m_currentOffset = currentOffset;
      m_averageOffset = averageOffset;
      m_currentOffsetError = currentOffsetError;
      m_averageOffsetError = averageOffsetError;
      m_fineSearch = true;
      m_recValid = true;
      m_detector = detector;
    }

    /**
     * Set number of tracks
     * @param numTracks number of tracks in acceptance of TOP
     * @param usedTracks number of tracks used for bunch reconstruction
     * @param nodEdx number of used tracks without dEdx information
     */
    void setNumTracks(int numTracks, int usedTracks, int nodEdx)
    {
      m_numTracks = numTracks;
      m_usedTracks = usedTracks;
      m_nodEdx = nodEdx;
    }

    /**
     * Sets number of system clock tics since last revo9 marker
     * @param revo9Counter counter state when L1 trigger is issued
     */
    void setRevo9Counter(unsigned short revo9Counter) {m_revo9Counter = revo9Counter;}

    /**
     * Sets chi2 value at minimum
     * @param minChi2 value at minimum
     */
    void setMinChi2(double minChi2) {m_minChi2 = minChi2;}

    /**
     * Add histogram
     * @param histo one dimensional histogram
     */
    void addHistogram(const TH1F& histo)
    {
      m_histograms.push_back(histo);
    }

    /**
     * Set simulated relative bunch number and time
     * @param bunchNo relative bunch number
     * @param time relative bunch time
     */
    void setSimulated(int bunchNo, double time)
    {
      m_simBunchNo = bunchNo;
      m_simTime = time;
      m_simValid = true;
    }

    /**
     * Sets reconstructed bucket number
     * @param bucketNumber bucket number
     */
    void setBucketNumber(int bucketNumber) {m_bucketNumber = bucketNumber;}

    /**
     * Sets reconstructed bucket fill status
     * @param isFilled fill status
     */
    void setBucketFillStatus(bool isFilled) {m_isBucketFilled = isFilled ? c_Filled : c_Empty;}

    /**
     * Returns reconstructed bunch number relative to L1 trigger signal at TOP
     * note: depends on the "look back" setting
     * @return bunch number relative to L1 trigger signal at TOP minus "look back"
     */
    int getBunchNo() const {return m_recBunchNo;}

    /**
     * Returns reconstructed bucket number within the ring
     * @param recBunchNo reconstructed bunch number relative to L1 trigger
     * @param revo9Count number of system clock tics since last revo9 marker at L1 trigger
     * @param offset calibrated offset [RF clock ticks]
     * @param RFBucketsPerRevolution number of RF buckets per beam revolution
     * @return bucket number
     */
    static int getBucketNumber(int recBunchNo, int revo9Count, int offset, int RFBucketsPerRevolution)
    {
      int bn = (recBunchNo + revo9Count * 4 - offset) % RFBucketsPerRevolution;
      if (bn < 0) bn += RFBucketsPerRevolution;
      return bn;
    }

    /**
     * Returns reconstructed bucket number within the ring
     * @param offset calibrated offset [RF clock ticks]
     * @param RFBucketsPerRevolution number of RF buckets per beam revolution
     * @return buncket number
     */
    int getBucketNumber(int offset, unsigned RFBucketsPerRevolution = 5120) const
    {
      return getBucketNumber(m_recBunchNo, m_revo9Counter, offset, RFBucketsPerRevolution);
    }

    /**
     * Returns reconstructed bucket number stored in private member
     * @return buncket number
     */
    int getBucketNumber() const {return m_bucketNumber;}

    /**
     * Returns bucket fill status
     * @return bucket fill status
     */
    EBucketStatus getBucketFillStatus() const {return m_isBucketFilled;}

    /**
     * Returns reconstructed bunch time relative to L1 trigger signal at TOP
     * (time to be subtracted to correct time of digits)
     * @return time relative to L1 trigger signal at TOP minus "look back"
     */
    double getTime() const {return m_recTime;}

    /**
     * Returns current offset to reconstructed bunch
     * @return offset
     */
    double getCurrentOffset() const {return m_currentOffset;}

    /**
     * Returns uncertainly of current offset
     * @return uncertainly
     */
    double getCurrentOffsetError() const {return m_currentOffsetError;}

    /**
     * Returns chi2 value at minimum
     * @return chi2 at minimum
     */
    double getMinChi2() const {return m_minChi2;}

    /**
     * Returns average offset to reconstructed bunch
     * @return offset
     */
    double getAverageOffset() const {return m_averageOffset;}

    /**
     * Returns uncertainly of average offset
     * @return uncertainly
     */
    double getAverageOffsetError() const {return m_averageOffsetError;}

    /**
     * Returns number of tracks in acceptance of TOP
     * @return number of tracks
     */
    int getNumTracks() const {return m_numTracks;}

    /**
     * Returns number of tracks used for bunch reconstruction
     * @return number of tracks
     */
    int getUsedTracks() const {return m_usedTracks;}

    /**
     * Returns number of used tracks without dEdx information
     * @return number of tracks
     */
    int getNodEdxTracks() const {return m_nodEdx;}

    /**
     * Returns histograms
     * @return 1D histograms
     */
    const std::vector<TH1F>& getHistograms() const {return m_histograms;}

    /**
     * Check if reconstructed return values are valid
     * @return true on valid
     */
    bool isReconstructed() const {return m_recValid;}

    /**
     * Check if fine search is done
     * @return true on fine search
     */
    bool isFineSearch() const {return m_fineSearch;}

    /**
     * Returns simulated bunch number (= bunch used for the event start time)
     * @return bunch number relative to the interaction
     */
    int getMCBunchNo() const {return m_simBunchNo;}

    /**
     * Returns simulated bunch time (= time used as event start time)
     * @return time relative to interaction time
     */
    double getMCTime() const {return m_simTime;}

    /**
     * Check if simulated return values are valid
     * @return true on valid
     */
    bool isSimulated() const {return m_simValid;}

    /**
     * Returns 127 MHz clock ticks since last revo9 marker
     * @return revo9counter
     */
    unsigned short getRevo9Counter() const {return m_revo9Counter;}

    /**
     * Returns detector component which provided the time seed
     * @return detector component
     */
    Const::EDetector getSeedingDetector() const {return m_detector;}

  private:

    int m_recBunchNo = 0; /**< reconstructed relative bunch number */
    float m_recTime = 0;  /**< reconstructed relative bunch time */
    float m_currentOffset = 0; /**< current offset */
    float m_averageOffset = 0; /**< running average of offset */
    float m_currentOffsetError = 0; /**< uncertainty of current offset */
    float m_averageOffsetError = 0; /**< uncertainty of running average of offset */
    int m_numTracks = 0;  /**< number of tracks in acceptance of TOP */
    int m_usedTracks = 0;  /**< number of tracks used for bunch reconstruction */
    int m_nodEdx = 0; /**< number of used tracks with no dEdx information */
    bool m_fineSearch = false; /**< if true, fine search was done */
    std::vector<TH1F> m_histograms;  /**< histograms, like chi2 vs t0 */
    bool m_recValid = false;  /**< status of rec */

    int m_simBunchNo = 0; /**< simulated relative bunch number */
    float m_simTime = 0;  /**< simulated relative bunch time */
    bool m_simValid = false;  /**< status of sim */

    unsigned short m_revo9Counter = 0xFFFF; /**< number of system clocks since last revo9 marker */
    float m_minChi2 = 0; /**< chi2 value at minimum */
    Const::EDetector m_detector = Const::invalidDetector; /**< component providing the time seed */

    int m_bucketNumber = c_Unknown;   /**< reconstructed bucket number */
    EBucketStatus m_isBucketFilled = c_Unknown; /**< reconstructed bucket status */

    ClassDef(TOPRecBunch, 7); /**< ClassDef */

  };


} // end namespace Belle2

