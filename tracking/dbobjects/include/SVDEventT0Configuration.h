/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TString.h>

namespace Belle2 {
  /**
   * This class store the configuration of the selections applied on the tracks used to compute the SVD EventT0
   */

  class SVDEventT0Configuration: public TObject {
  public:
    /**
     * Default constructor
     */
    SVDEventT0Configuration(const TString& uniqueID = "")
      : m_uniqueID(uniqueID)
      , m_selectTracksFromIP(true)
      , m_minimumPtSelection(0.25)
      , m_absPzSelection(0.0)
      , m_absD0Selection(0.5)
      , m_absZ0Selection(2.0)
    {};

    /**
     * Get payload name
     */
    std::string get_name() const {return s_name;}

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /**
     * Return the boolean used to specify whether apply the selection based
     * on the absolute values of d0 and z0, to select tracks from the IP
     * to compute SVDEventT0, or not
     */
    bool getSelectTracksFromIP() const { return m_selectTracksFromIP; };

    /**
     * Return the cut on minimum transverse momentum pt for RecoTrack selection
     */
    double getMinimumPtSelection() const { return m_minimumPtSelection; };

    /**
     * Return the cut on minimum absolute value of the longitudinal momentum, abs(pz), for RecoTrack selection
     */
    double getAbsPzSelection() const { return m_absPzSelection; };

    /**
     * Return the cut on maximum absolute value of the d0 for RecoTrack selection
     */
    double getAbsD0Selection() const { return m_absD0Selection; };

    /**
     * Return cut on maximum absolute value of the z0 for RecoTrack selection
     */
    double getAbsZ0Selection() const { return m_absZ0Selection; };

    /**
     * Set the boolean used to specify whether apply the selection based
     * on the absolute values of d0 and z0, to select tracks from the IP
     * to compute SVDEventT0, or not
     */
    void setSelectTracksFromIP(bool selectTracksFromIP) { m_selectTracksFromIP = selectTracksFromIP; };

    /**
     * Set the cut on minimum transverse momentum pt for RecoTrack selection
     */
    void setMinimumPtSelection(double minimumPtSelection) { m_minimumPtSelection = minimumPtSelection; };

    /**
     * Set the cut on minimum absolute value of the longitudinal momentum, abs(pz), for RecoTrack selection
     */
    void setAbsPzSelection(double absPzSelection) { m_absPzSelection = absPzSelection; };

    /**
     * Set the cut on maximum absolute value of the d0 for RecoTrack selection
     */
    void setAbsD0Selection(double absD0Selection) { m_absD0Selection = absD0Selection; };

    /**
     * Set cut on maximum absolute value of the z0 for RecoTrack selection
     */
    void setAbsZ0Selection(double absZ0Selection) { m_absZ0Selection = absZ0Selection; };

  private:

    /** name of the payload */
    static std::string s_name;

    /** unique identifier of the SVD EventT0 configuration payload */
    TString m_uniqueID;

    bool m_selectTracksFromIP; /**< Apply the selection based on the absolute values of d0 */
    double m_minimumPtSelection; /**< Cut on pt for RecoTrack selection */
    double m_absPzSelection; /**< Cut on abs(pz) for RecoTrack selection */
    double m_absD0Selection; /**< Cut on abs(d0), in cm, for RecoTrack selection */
    double m_absZ0Selection; /**< Cut on abs(z0), in cm, for RecoTrack selection */

    ClassDef(SVDEventT0Configuration, 1); /**< needed by root*/

  };

}


