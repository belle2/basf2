/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>
#include <vector>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * Class to store photon pixel yields for PMT ageing studies, and equalized alpha ratios for finding optically decoupled PMT's.
   */
  class TOPCalPhotonYields: public TObject {

  public:

    /**
     * Default constructor.
     */
    TOPCalPhotonYields() {}

    /**
     * Sets unix time stamp.
     * @param utime average unix time stamp (in seconds)
     * @param utimeStd standard deviation (in seconds)
     */
    void setTimeStamp(unsigned utime, unsigned utimeStd)
    {
      m_utime = utime;
      m_utimeStd = utimeStd;
    }

    /**
     * Sets the data of a given slot. The data are given as 1D histograms of 512 bins that correspond to pixelID's.
     * Note: the setter must be called in the ascending order of slot ID's, e.g slot = 1, 2, ... , 16
     * @param slot slot ID
     * @param photonYields pixel photon yields
     * @param backgroundYields pixel background yields
     * @param alphaRatio equalized alpha ratio
     * @param activePixels active pixels
     * @param pulseHeights pixel pulse-heights
     * @param muonZ z-distribution of tracks
     */
    void set(int slot, const TH1F* photonYields, const TH1F* backgroundYields, const TH1F* alphaRatio,
             const TH1F* activePixels, const TH2F* pulseHeights, const TH1F* muonZ);

    /**
     * Returns average unix time stamp of events used to make histograms stored in this class.
     * @return unix time stamp (in seconds)
     */
    unsigned getTimeStamp() const {return m_utime;}

    /**
     * Returns standard deviation of unix time stamps of events used to make histograms stored in this class.
     * @return standard deviation (in seconds)
     */
    unsigned getTimeStampStd() const {return m_utimeStd;}

    /**
     * Returns a 2D histogram of photon pixel yields.
     * These are background subtracted and inefficiency corrected number of photons per track in a pixel.
     * @param slot slot ID
     * @return pixel image of photon yields (nullptr if N/A)
     */
    const TH2F* getPhotonYields(int slot) const;

    /**
     * Returns a 2D histogram of background pixel yields.
     * These are inefficiency corrected number of background hits per track in a pixel.
     * @param slot slot ID
     * @return pixel image of photon yields (nullptr if N/A)
     */
    const TH2F* getBackgroundYields(int slot) const;

    /**
     * Returns a 2D histogram of equalized pixel alpha ratio.
     * For optically well coupled pixels this ratio is around 1 and it is around 0.5 for those which are not.
     * @param slot slot ID
     * @return pixel image of alpha ratio (nullptr if N/A)
     */
    const TH2F* getAlphaRatio(int slot) const;

    /**
     * Returns a 2D histogram of active pixels.
     * @param slot slot ID
     * @return pixel image of active pixels (nullptr if N/A)
     */
    const TH2F* getActivePixels(int slot) const;

    /**
     * Returns a 2D histogram of pixel pulse-heights.
     * @param slot slot ID
     * @return pixel pulse-heights (nullptr if N/A)
     */
    const TH2F* getPulseHeights(int slot) const;

    /**
     * Returns z distribution of tracks used to determine pixel yields.
     * @param slot slot ID
     * @return z distribution in local frame (nullptr if N/A)
     */
    const TH1F* getMuonZ(int slot) const;

  private:

    /**
     * Number of pixel rows and columns
     */
    enum EPixels {
      c_numRows = 8, /**< number of pixel rows */
      c_numCols = 64 /**< number of pixel columns */
    };

    /**
     * Copy content of 1D histogram into 2D histogram.
     * @param input 1D histogram
     * @param output 2D histogram
     */
    void copyContent(const TH1F* input, TH2F& output);

    unsigned m_utime = 0; /**< average unix time in seconds */
    unsigned m_utimeStd = 0; /**< standard deviation of unix time in seconds */
    std::vector<TH2F> m_photonYields;  /**< photon yields per pixel (index = slot - 1) */
    std::vector<TH2F> m_backgroundYields;     /**< background yields per pixel (index = slot - 1) */
    std::vector<TH2F> m_alphaRatio;    /**< equalized alpha ratio per pixel (index = slot - 1) */
    std::vector<TH2F> m_activePixels;  /**< active pixels (index = slot - 1) */
    std::vector<TH2F> m_pulseHeights;  /**< pixel pulse-heights (index = slot - 1) */
    std::vector<TH1F> m_muonZ;         /**< local z distribution of tracks (index = slot - 1) */

    ClassDef(TOPCalPhotonYields, 2); /**< ClassDef */

  };

}
