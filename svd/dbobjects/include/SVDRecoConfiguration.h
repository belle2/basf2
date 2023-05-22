/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include "TObject.h"
#include "TString.h"

namespace Belle2 {
  /**
   * This class store the reconstruction configuration of SVD
   */

  class SVDRecoConfiguration: public TObject {
  public:
    /**
    * Default constructor
    */
    SVDRecoConfiguration(const TString& uniqueID = "")
      : m_uniqueID(uniqueID)
      , m_timeRecoWith6Samples("CoG6")
      , m_timeRecoWith3Samples("CoG3")
      , m_chargeRecoWith6Samples("MaxSample")
      , m_chargeRecoWith3Samples("MaxSample")
      , m_positionRecoWith6Samples("CoG")
      , m_positionRecoWith3Samples("CoG")
      , m_stripTimeRecoWith6Samples("CoG6")
      , m_stripTimeRecoWith3Samples("CoG3")
      , m_stripChargeRecoWith6Samples("MaxSample")
      , m_stripChargeRecoWith3Samples("MaxSample")
      , m_assignSVDGroupIn6Samples(false)
      , m_assignSVDGroupIn3Samples(false)
      , m_useSVDGroupInfoInSPfor6Samples(false)
      , m_useSVDGroupInfoInSPfor3Samples(false)
    {};

    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster time computation in 6-sample acquisition mode
     **/
    TString getTimeRecoWith6Samples() const { return m_timeRecoWith6Samples; };
    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster time computation in 3-sample acquisition mode
     **/
    TString getTimeRecoWith3Samples() const { return m_timeRecoWith3Samples; };

    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster charge computation in 6-sample acquisition mode
     **/
    TString getChargeRecoWith6Samples() const { return m_chargeRecoWith6Samples; };
    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster charge computation in 3-sample acquisition mode
     **/
    TString getChargeRecoWith3Samples() const { return m_chargeRecoWith3Samples; };

    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster position computation in 6-sample acquisition mode
     **/
    TString getPositionRecoWith6Samples() const { return m_positionRecoWith6Samples; };
    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the cluster position computation in 3-sample acquisition mode
     **/
    TString getPositionRecoWith3Samples() const { return m_positionRecoWith3Samples; };


    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the strip time computation in 6-sample acquisition mode
     **/
    TString getStripTimeRecoWith6Samples() const { return m_stripTimeRecoWith6Samples; };
    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the strip time computation in 3-sample acquisition mode
     **/
    TString getStripTimeRecoWith3Samples() const { return m_stripTimeRecoWith3Samples; };

    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the strip charge computation in 6-sample acquisition mode
     **/
    TString getStripChargeRecoWith6Samples() const { return m_stripChargeRecoWith6Samples; };
    /**
     * Returns the TString corresponding to the algorithm to be used
     * for the strip charge computation in 3-sample acquisition mode
     **/
    TString getStripChargeRecoWith3Samples() const { return m_stripChargeRecoWith3Samples; };


    /**
     * Returns the parameter which turns On/Off SVD cluster grouping
     * in 3/6-sample acquisition mode
     */
    Bool_t isSVDTimeGroupingEnabled(const Int_t& mode) const
    {
      if (mode == 6)
        return m_assignSVDGroupIn6Samples;
      else if (mode == 3)
        return m_assignSVDGroupIn3Samples;
      else
        return false;
    }
    /**
     * Returns the parameter which turns On/Off usage of SVD cluster grouping
     * in the SVDSpacePointCreator for 3/6-sample acquisition mode
     */
    Bool_t isSVDGroupInfoUsedInSPCreator(const Int_t& mode) const
    {
      if (mode == 6)
        return m_useSVDGroupInfoInSPfor6Samples;
      else if (mode == 3)
        return m_useSVDGroupInfoInSPfor3Samples;
      else
        return false;
    }

    /**
     * Set the algorithm to be used for
     * cluster time computation in 6-sample acquisition mode
     */
    void setTimeRecoWith6Samples(TString timeRecoWith6Samples)
    {
      m_timeRecoWith6Samples = TString(timeRecoWith6Samples);
    }
    /**
     * Set the algorithm to be used for
     * cluster time computation in 3-sample acquisition mode
     */
    void setTimeRecoWith3Samples(TString timeRecoWith3Samples)
    {
      m_timeRecoWith3Samples = TString(timeRecoWith3Samples);
    }

    /**
     * Set the algorithm to be used for
     * cluster charge computation in 6-sample acquisition mode
     */
    void setChargeRecoWith6Samples(TString chargeRecoWith6Samples)
    {
      m_chargeRecoWith6Samples = TString(chargeRecoWith6Samples);
    }
    /**
     * Set the algorithm to be used for
     * cluster charge computation in 3-sample acquisition mode
     */
    void setChargeRecoWith3Samples(TString chargeRecoWith3Samples)
    {
      m_chargeRecoWith3Samples = TString(chargeRecoWith3Samples);
    }

    /**
     * Set the algorithm to be used for
     * cluster position computation in 6-sample acquisition mode
     */
    void setPositionRecoWith6Samples(TString positionRecoWith6Samples)
    {
      m_positionRecoWith6Samples = TString(positionRecoWith6Samples);
    }
    /**
     * Set the algorithm to be used for
     * cluster position computation in 3-sample acquisition mode
     */
    void setPositionRecoWith3Samples(TString positionRecoWith3Samples)
    {
      m_positionRecoWith3Samples = TString(positionRecoWith3Samples);
    }


    /**
     * Set the algorithm to be used for
     * strip time computation in 6-sample acquisition mode
     */
    void setStripTimeRecoWith6Samples(TString timeStripRecoWith6Samples)
    {
      m_stripTimeRecoWith6Samples = TString(timeStripRecoWith6Samples);
    }
    /**
     * Set the algorithm to be used for
     * strip time computation in 3-sample acquisition mode
     */
    void setStripTimeRecoWith3Samples(TString timeStripRecoWith3Samples)
    {
      m_stripTimeRecoWith3Samples = TString(timeStripRecoWith3Samples);
    }

    /**
     * Set the algorithm to be used for
     * strip charge computation in 6-sample acquisition mode
     */
    void setStripChargeRecoWith6Samples(TString chargeStripRecoWith6Samples)
    {
      m_stripChargeRecoWith6Samples = TString(chargeStripRecoWith6Samples);
    }
    /**
     * Set the algorithm to be used for
     * strip charge computation in 3-sample acquisition mode
     */
    void setStripChargeRecoWith3Samples(TString chargeStripRecoWith3Samples)
    {
      m_stripChargeRecoWith3Samples = TString(chargeStripRecoWith3Samples);
    }

    /**
     * Set the parameter which turns On/Off SVD cluster grouping
     * in 3/6-sample acquisition mode
     */
    void setStateOfSVDTimeGrouping(const Int_t& numberOfAcquiredSamples, const Bool_t& state = false)
    {
      if (numberOfAcquiredSamples == 3)
        m_assignSVDGroupIn3Samples = state;
      else if (numberOfAcquiredSamples == 6)
        m_assignSVDGroupIn6Samples = state;
    }
    /**
     * Set the parameter which turns On/Off usage of SVD cluster grouping
     * in the SVDSpacePointCreator for 3/6-sample acquisition mode
     */
    void setUseOfSVDGroupInfoInSPCreator(const Int_t& numberOfAcquiredSamples, const Bool_t& state = false)
    {
      if (numberOfAcquiredSamples == 3)
        m_useSVDGroupInfoInSPfor3Samples = state;
      else if (numberOfAcquiredSamples == 6)
        m_useSVDGroupInfoInSPfor6Samples = state;
    }

    /**
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    /** name of the payload */
    static std::string name;

  private:

    /** unique identifier of the SVD reconstruction configuration payload */
    TString m_uniqueID;

    /** cluster time reconstruction algorithm used in 6-sample acquisition mode */
    TString m_timeRecoWith6Samples;
    /** cluster time reconstruction algorithm used in 3-sample acquisition mode */
    TString m_timeRecoWith3Samples;
    /** cluster charge reconstruction algorithm used in 6-sample acquisition mode */
    TString m_chargeRecoWith6Samples;
    /** cluster charge reconstruction algorithm used in 3-sample acquisition mode */
    TString m_chargeRecoWith3Samples;

    /** cluster position reconstruction algorithm used in 6-sample acquisition mode */
    TString m_positionRecoWith6Samples;
    /** cluster position reconstruction algorithm used in 3-sample acquisition mode */
    TString m_positionRecoWith3Samples;


    /** strip time reconstruction algorithm used in 6-sample acquisition mode */
    TString m_stripTimeRecoWith6Samples;
    /** strip time reconstruction algorithm used in 3-sample acquisition mode */
    TString m_stripTimeRecoWith3Samples;
    /** strip charge reconstruction algorithm used in 6-sample acquisition mode */
    TString m_stripChargeRecoWith6Samples;
    /** strip charge reconstruction algorithm used in 3-sample acquisition mode */
    TString m_stripChargeRecoWith3Samples;

    /** Assign groupId to SVD clusters in 6-sample acquisition mode */
    Bool_t m_assignSVDGroupIn6Samples;
    /** Assign groupId to SVD clusters in 3-sample acquisition mode */
    Bool_t m_assignSVDGroupIn3Samples;
    /** Use SVD clusters groupId in SVDSpacePointCreator for 6-sample acquisition mode */
    Bool_t m_useSVDGroupInfoInSPfor6Samples;
    /** Use SVD clusters groupId in SVDSpacePointCreator for 3-sample acquisition mode */
    Bool_t m_useSVDGroupInfoInSPfor3Samples;

    ClassDef(SVDRecoConfiguration, 2); /**< needed by root*/

  };

}
