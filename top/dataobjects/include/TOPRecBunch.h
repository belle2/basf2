/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPRECBUNCH_H
#define TOPRECBUNCH_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /**
   * Class to store results of TOPBunchFinder
   */

  class TOPRecBunch : public RelationsObject {
  public:

    /**
     * Default constructor
     */
    TOPRecBunch():
      m_recBunchNo(0),
      m_recTime(0),
      m_numTracks(0),
      m_usedTracks(0),
      m_recValid(false),
      m_simBunchNo(0),
      m_simTime(0),
      m_simValid(false)
    {}

    /**
     * Set reconstructed relative bunch number and time
     * @param bunchNo relative bunch number
     * @param time relative bunch time
     * @param numTracks number of tracks in acceptance of TOP
     * @param usedTracks number of tracks used in bunch reconstruction
     */
    void setReconstructed(int bunchNo, double time, int numTracks, int usedTracks) {
      m_recBunchNo = bunchNo;
      m_recTime = time;
      m_numTracks = numTracks;
      m_usedTracks = usedTracks;
      m_recValid = true;
    }

    /**
     * Append log likelihood
     * @param logL log likelihood
     */
    void addLogL(double logL) {
      m_logL.push_back(logL);
    }

    /**
     * Set simulated relative bunch number and time
     * @param bunchNo relative bunch number
     * @param time relative bunch time
     */
    void setSimulated(int bunchNo, double time) {
      m_simBunchNo = bunchNo;
      m_simTime = time;
      m_simValid = true;
    }

    /**
     * Return reconstructed bunch number (= bunch used for the event start time)
     * @return bunch number relative to the interaction
     */
    int getBunchNo() const {return m_recBunchNo;}

    /**
     * Return reconstructed bunch time (time to be added to correct time of digits)
     * @return time relative to interaction time
     */
    float getTime() const {return m_recTime;}

    /**
     * Return number of tracks in acceptance of TOP
     * @return number of tracks
     */
    int getNumTracks() const {return m_numTracks;}

    /**
     * Return number of tracks used in the bunch number reconstruction
     * @return number of tracks
     */
    int getUsedTracks() const {return m_usedTracks;}

    /**
     * Return vector of log likelihoods for the bunches
     * @return log likelihoods
     */
    const std::vector<float>& getLogL() const {return m_logL;}

    /**
     * Check if reconstructed return values are valid
     * @return true on valid
     */
    bool isReconstructed() const {return m_recValid;}

    /**
     * Return simulated bunch number (= bunch used for the event start time)
     * @return bunch number relative to the interaction
     */
    int getMCBunchNo() const {return m_simBunchNo;}

    /**
     * Return simulated bunch time (= time used as event start time)
     * @return time relative to interaction time
     */
    float getMCTime() const {return m_simTime;}

    /**
     * Check if simulated return values are valid
     * @return true on valid
     */
    bool isSimulated() const {return m_simValid;}


  private:

    int m_recBunchNo; /**< reconstructed relative bunch number */
    float m_recTime;  /**< reconstructed relative bunch time */
    int m_numTracks;  /**< number of tracks in acceptance of TOP */
    int m_usedTracks;  /**< number of tracks used for bunch reconstruction */
    std::vector<float> m_logL; /**< log likelihoods of bunches */
    bool m_recValid;  /**< status of rec */
    int m_simBunchNo; /**< simulated relative bunch number */
    float m_simTime;  /**< simulated relative bunch time */
    bool m_simValid;  /**< status of sim */

    ClassDef(TOPRecBunch, 1); /**< ClassDef */

  };


} // end namespace Belle2

#endif
