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
   * Database object for Muid.
   */
  class MuidParameters: public TObject {

    /* identification id for layer profile*/
    typedef unsigned short profileID;

    /* identification id for ReducedChiSquaredPDF*/
    typedef unsigned short pdfID;

  public:

    /**
    * Default constructor
    */
    MuidParameters() {}

    /**
    * layer profile ID
    * bit#0 - 3: 4bits for hypothesis, 0: Positron, 1: Electron, 2:Deuteron, 3: Antideuteron: 4: Proton: 5: Antiproton 6: PionPlus
    * 7: PionMinus 8: KaonPlus 9: KaonMinus 10: MuonPlus 11: MuonMinus
    * bit#4 - 10: 7bits for outcome
    * bit#11 - 14: 4bits for lastLayer
    */
    profileID getProfileID(int hypothesis, int outcome, int lastLayer) const
    {
      profileID id = lastLayer + (outcome << 4) + (hypothesis << 11);
      return id;
    }

    /*
    * PDF ID
    * bit#0 - 3:  4bits for hypothesis, 0: Positron, 1: Electron, 2:Deuteron, 3: Antideuteron: 4: Proton: 5: Antiproton 6: PionPlus
    * 7: PionMinus 8: KaonPlus 9: KaonMinus 10: MuonPlus 11: MuonMinus 12
    * bit# 4-5: 2bits 0: BarrelAndEndcap 1: BarrelOnly 2: EndcapOnly
    * bit#6-11: 6bits for DegreesOfFreedom.
    */
    pdfID getPdfID(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      pdfID  id = degreesOfFreedom + (barrelorEndcap << 6) + (hypothesis << 8);
      return id;
    }

    //! set Layer profile
    void setLayerProfile(const profileID id, const std::vector<double>& params)
    {
      m_layerProfile.insert(std::pair<profileID, std::vector<double>>(id, params));
    }

    //! set Layer profile
    void setLayerProfile(int hypothesis, int outcome, int lastLayer, const std::vector<double>& params)
    {
      const profileID id =  getProfileID(hypothesis, outcome, lastLayer);
      setLayerProfile(id, params);
    }

    //! set PDF
    void setPDF(const pdfID id, const std::vector<double>& params)
    {
      m_transversePDF.insert(std::pair<pdfID, std::vector<double>>(id, params));
    }

    //! set PDF
    void setPDF(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const std::vector<double>& params)
    {
      pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setPDF(id, params);
    }

    //! set PDF threshold
    void setThreshold(const pdfID id, const double threshold)
    {
      m_threshold.insert(std::pair<pdfID, double>(id, threshold));
    }

    //! set PDF threshold
    void setThreshold(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double threshold)
    {
      pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setThreshold(id, threshold);
    }

    //! set PDF ScaleY
    void setScaleY(const pdfID id, const double scaleY)
    {
      m_scaleY.insert(std::pair<pdfID, double>(id, scaleY));
    }

    //! set PDF ScaleY
    void setScaleY(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double scaleY)
    {
      pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setScaleY(id, scaleY);
    }

    //! set PDF ScaleX
    void setScaleX(const pdfID id, const double scaleX)
    {
      m_scaleX.insert(std::pair<pdfID, double>(id, scaleX));
    }

    //! set PDF ScaleX
    void setScaleX(int hypothesis, int barrelorEndcap, int degreesOfFreedom, const double scaleX)
    {
      pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      setScaleX(id, scaleX);
    }

    //! get layer Profile
    const std::vector<double>& getProfile(const profileID id) const
    {
      std::map<profileID, std::vector<double>>::const_iterator it = m_layerProfile.find(id);
      if (it != m_layerProfile.end()) {
        return it->second;
      } else {
        B2FATAL("LayerProfile for Muid not found !");
      }
    }

    //! get layer Profile
    const std::vector<double>& getProfile(int hypothesis, int outcome, int lastLayer) const
    {
      const profileID id =  getProfileID(hypothesis, outcome, lastLayer);
      return getProfile(id);
    }

    //! get transverse PDF
    const std::vector<double>& getPDF(const pdfID id) const
    {
      std::map<pdfID, std::vector<double>>::const_iterator it = m_transversePDF.find(id);
      if (it != m_transversePDF.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF for Muid not found !");
      }
    }

    //! get transverse PDF
    const std::vector<double>& getPDF(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getPDF(id);
    }

    //! get PDF Threshold
    double getThreshold(const pdfID id) const
    {
      std::map<pdfID, double>::const_iterator it = m_threshold.find(id);
      if (it != m_threshold.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF Threshold for Muid not found !");
      }
    }

    //! get PDF Threshold
    double getThreshold(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getThreshold(id);
    }

    //! get PDF ScaleY
    double getScaleY(const pdfID id) const
    {
      std::map<pdfID, double>::const_iterator it = m_scaleY.find(id);
      if (it != m_scaleY.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF ScaleY for Muid not found !");
      }
    }

    //! get PDF ScaleY
    double getScaleY(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getScaleY(id);
    }

    //! get PDF ScaleX
    double getScaleX(const pdfID id) const
    {
      std::map<pdfID, double>::const_iterator it = m_scaleX.find(id);
      if (it != m_scaleX.end()) {
        return it->second;
      } else {
        B2FATAL("Transverse PDF ScaleX for Muid not found !");
      }
    }

    //! get PDF ScaleX
    double getScaleX(int hypothesis, int barrelorEndcap, int degreesOfFreedom) const
    {
      const pdfID id = getPdfID(hypothesis, barrelorEndcap, degreesOfFreedom);
      return getScaleX(id);
    }

  private:

    /* layerProfile dictionary*/
    std::map<profileID, std::vector<double>> m_layerProfile;

    /* transversePDF dictionary Histogram*/
    std::map<pdfID, std::vector<double>> m_transversePDF;

    /* transversePDF Threshold*/
    std::map<pdfID, double> m_threshold;

    /* transversePDF ScaleY*/
    std::map<pdfID, double> m_scaleY;

    /* transversePDF ScaleX*/
    std::map<pdfID, double> m_scaleX;

    /* ClassDef */
    ClassDef(MuidParameters, 1);

  };
} // end namespace Belle2
