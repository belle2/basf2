/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      , m_timeRecoWith6Samples("not set")
      , m_timeRecoWith3Samples("not set")
      , m_chargeRecoWith6Samples("not set")
      , m_chargeRecoWith3Samples("not set")
      , m_positionRecoWith6Samples("not set")
      , m_positionRecoWith3Samples("not set")
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
     * Get the unique ID  of the calibration
     */
    TString get_uniqueID() const {return m_uniqueID;}

    static std::string name;

  private:

    TString m_uniqueID;   /**< Add a string as unique identifier of the SVD reconstruction algorithms*/

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

    ClassDef(SVDRecoConfiguration, 1); /**< needed by root*/

  };

}
