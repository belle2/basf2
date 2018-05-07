/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: KLM group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <framework/logging/Logger.h>
#include <map>
#include <array>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object to record parameters for Muon identification.
   */
  class MuidParameters: public TObject {

  public:

    /**
    * Default constructor
    */
    MuidParameters() = default;

    /**
    * get the unique identification id for given hypothesis, outcome state and number of layer
    * bit#0 - 3: 4bits for hypothesis, 0: Positron, 1: Electron, 2:Deuteron, 3: Antideuteron: 4: Proton: 5: Antiproton 6: PionPlus
    * 7: PionMinus 8: KaonPlus 9: KaonMinus 10: MuonPlus 11: MuonMinus
    * bit#4 - 10: 7bits for outcome type
    * bit#11 - 14: 4bits for lastLayer id
    */
    int getProfileID(int hypothesis, int outcome, int lastLayer) const
    {
      int id = lastLayer + (outcome << c_LastLayerBits) + (hypothesis << (c_LastLayerBits + c_OutcomeTypeBits));
      return id;
    }

    /*
    * get unique identification id for given hypothesis, detector component and number of degree freedom
    * bit#0 - 3:  4bits for hypothesis, 0: Positron, 1: Electron, 2:Deuteron, 3: Antideuteron: 4: Proton: 5: Antiproton 6: PionPlus
    * 7: PionMinus 8: KaonPlus 9: KaonMinus 10: MuonPlus 11: MuonMinus 12
    * bit#4 - 5: 2bits 0: BarrelAndEndcap 1: BarrelOnly 2: EndcapOnly
    * bit#6 - 11: 6bits for DegreesOfFreedom.
    */
    int getPdfID(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      int id = degreesOfFreedom + (barrelorEndcap << c_NumDegreesOfFreedomBits) + (hypothesis << (c_NumDegreesOfFreedomBits +
               c_IsForwardBits));
      return id;
    }

    //! set the (longitudinal) probability density function for a given id
    void setLayerProfile(const int id, const std::vector<double>& params)
    {
      m_layerProfile.insert(std::pair<int, std::vector<double>>(id, params));
    }

    //! set the (longitudinal) probability density function for specific hypothesis, outcome state and laterLayer
    void setLayerProfile(int hypothesis, int outcome, int lastLayer, const std::vector<double>& params)
    {
      const int id =  getProfileID(hypothesis, outcome, lastLayer);
      setLayerProfile(id, params);
    }

    //! set the reduced chi-squared (transverse) probability density function for a given id
    void setPDF(const int id, const std::vector<double>& params)
    {
      m_transversePDF.insert(std::pair<int, std::vector<double>>(id, params));
    }

    //! set the reduced chi-squared (transverse) probability density function for specific hypothesis, detector and ndof
    void setPDF(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const std::vector<double>& params)
    {
      int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setPDF(id, params);
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): threshold for a given id
    void setThreshold(const int id, const double threshold)
    {
      m_threshold.insert(std::pair<int, double>(id, threshold));
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): threshold for specific hypothesis, detector and ndof
    void setThreshold(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double threshold)
    {
      int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setThreshold(id, threshold);
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): vertical scale for a given id
    void setScaleY(const int id, const double scaleY)
    {
      m_scaleY.insert(std::pair<int, double>(id, scaleY));
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): vertical scale for specific hypothesis detector and ndof
    void setScaleY(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double scaleY)
    {
      int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setScaleY(id, scaleY);
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): horizontal scale ~ 1 for a given id
    void setScaleX(const int id, const double scaleX)
    {
      m_scaleX.insert(std::pair<int, double>(id, scaleX));
    }

    //! set the reduced chi-squared (transverse) probability density function (analytical): horizontal scale ~ 1 for specific hypothesis detector and ndof
    void setScaleX(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double scaleX)
    {
      int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setScaleX(id, scaleX);
    }

    //! get the (longitudinal) probability density function for a given id
    const std::vector<double>& getProfile(const int id) const
    {
      std::map<int, std::vector<double>>::const_iterator it = m_layerProfile.find(id);
      if (it != m_layerProfile.end()) {
        return it->second;
      } else {
        B2FATAL("LayerProfile for Muid not found !");
      }
    }

    //! get the (longitudinal) probability density function for specific hypothesis, outcome state and laterLaye
    const std::vector<double>& getProfile(int hypothesis, int outcome, int lastLayer) const
    {
      const int id =  getProfileID(hypothesis, outcome, lastLayer);
      return getProfile(id);
    }

    //! get the reduced chi-squared (transverse) probability density function for a given id
    const std::vector<double>& getPDF(const int id) const
    {
      std::map<int, std::vector<double>>::const_iterator it = m_transversePDF.find(id);
      if (it != m_transversePDF.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF for Muid not found !");
      }
    }

    //! get the reduced chi-squared (transverse) probability density function for specific hypothesis, detector and ndof
    const std::vector<double>& getPDF(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getPDF(id);
    }

    //! get the reduced chi-squared (transverse) probability density function (analytical): threshold for a given id
    double getThreshold(const int id) const
    {
      std::map<int, double>::const_iterator it = m_threshold.find(id);
      if (it != m_threshold.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF Threshold for Muid not found !");
      }
    }

    //! get the reduced chi-squared (transverse) probability density function (analytical): threshold for specific hypothesis detector and ndof
    double getThreshold(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getThreshold(id);
    }

    //! get the reduced chi-squared (transverse) probability density function (analytical): vertical scale for a given id
    double getScaleY(const int id) const
    {
      std::map<int, double>::const_iterator it = m_scaleY.find(id);
      if (it != m_scaleY.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF ScaleY for Muid not found !");
      }
    }

    //! get the reduced chi-squared (transverse) probability density function (analytical): vertical scale for specific hypothesis, detector and ndof
    double getScaleY(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getScaleY(id);
    }


    //! get the reduced chi-squared (transverse) probability density function (analytical): horizontal scale for a given id
    double getScaleX(const int id) const
    {
      std::map<int, double>::const_iterator it = m_scaleX.find(id);
      if (it != m_scaleX.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF ScaleX for Muid not found !");
      }
    }

    //! get the reduced chi-squared (transverse) probability density function (analytical): horizontal scale for specific hypothesis detector and ndof
    double getScaleX(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const int id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getScaleX(id);
    }

    //! bitsset for identifying a state of track in KLM
    enum {
      c_OutcomeTypeBits = 7,
      c_LastLayerBits = 4,
      c_IsForwardBits = 2,
      c_NumDegreesOfFreedomBits = 6
    };


  private:

    /* Per-layer (longitudinal) probability density function */
    std::map<int, std::vector<double>> m_layerProfile;

    /* Reduced chi-squared (transverse) probability density function */
    std::map<int, std::vector<double>> m_transversePDF;

    /* Reduced chi-squared (transverse) probability density function (analytical): threshold */
    std::map<int, double> m_threshold;

    /* Reduced chi-squared (transverse) probability density function (analytical): vertical scale */
    std::map<int, double> m_scaleY;

    /* Reduced chi-squared (transverse) probability density function (analytical): horizontal */
    std::map<int, double> m_scaleX;

    /* ClassDef */
    ClassDef(MuidParameters, 1);

  };
} // end namespace Belle2
