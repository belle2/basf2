/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

/* KLM headers. */
#include <klm/muid/MuidElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <map>

namespace Belle2 {

  /**
   * Database object used to store the parameters for KLM likelihood computation.
   */
  class KLMLikelihoodParameters : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMLikelihoodParameters()
    {
    }

    /**
     * Destructor.
     */
    ~KLMLikelihoodParameters()
    {
    }

    /**
     * Set the longitudinal probability density function for specific hypothesis, outcome and last layer.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation.
     * @param[in] params     Likelihood parameters.
     */
    void setLongitudinalPDF(int hypothesis, int outcome, int lastLayer, const std::vector<double>& params)
    {
      const int id =  MuidElementNumbers::getLongitudinalID(hypothesis, outcome, lastLayer);
      setLongitudinalPDF(id, params);
    }

    /**
     * Set the transverse probability density function for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] params           Likelihood parameters.
     */
    void setTransversePDF(int hypothesis, int detector, int degreesOfFreedom, const std::vector<double>& params)
    {
      int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransversePDF(id, params);
    }

    /**
     * Set the transverse probability density function (analytical): threshold for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] threshold        Transverse threshold.
     */
    void setTransverseThreshold(int hypothesis, int detector, int degreesOfFreedom, const double threshold)
    {
      int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseThreshold(id, threshold);
    }

    /**
     * Set the transverse probability density function (analytical): horizontal scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] scaleX           Horizontal scale.
     */
    void setTransverseScaleX(int hypothesis, int detector, int degreesOfFreedom, const double scaleX)
    {
      int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseScaleX(id, scaleX);
    }

    /**
     * Set the transverse probability density function (analytical): vertical scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     * @param[in] scaleY           Vertical scale.
     */
    void setTransverseScaleY(int hypothesis, int detector, int degreesOfFreedom, const double scaleY)
    {
      int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      setTransverseScaleY(id, scaleY);
    }

    /**
     * Get the longitudinal probability density function for specific hypothesis, outcome and last layer.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation.
     */
    const std::vector<double>& getLongitudinalPDF(int hypothesis, int outcome, int lastLayer) const
    {
      const int id = MuidElementNumbers::getLongitudinalID(hypothesis, outcome, lastLayer);
      return getLongitudinalPDF(id);
    }

    /**
     * Get the transverse probability density function for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    const std::vector<double>& getTransversePDF(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransversePDF(id);
    }

    /**
     * Get the transverse probability density function (analytical): threshold for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseThreshold(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseThreshold(id);
    }

    /**
     * Get the transverse probability density function (analytical): horizontal scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseScaleX(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseScaleX(id);
    }

    /**
     * Get the transverse probability density function (analytical): vertical scale for specific hypothesis, detector and degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    double getTransverseScaleY(int hypothesis, int detector, int degreesOfFreedom) const
    {
      const int id = MuidElementNumbers::getTransverseID(hypothesis, detector, degreesOfFreedom);
      return getTransverseScaleY(id);
    }

  private:

    /**
     * Set the longitudinal probability density function for a given identifier.
     * @param[in] id     Longitudinal identifier.
     * @param[in] params Likelihood parameters.
     */
    void setLongitudinalPDF(const int id, const std::vector<double>& params)
    {
      m_LongitudinalPDF.insert(std::pair<int, std::vector<double>>(id, params));
    }

    /**
     * Set the transverse probability density function for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] params Likelihood parameters.
     */
    void setTransversePDF(const int id, const std::vector<double>& params)
    {
      m_TransversePDF.insert(std::pair<int, std::vector<double>>(id, params));
    }

    /**
     * Set the transverse probability density function (analytical): threshold for a given identifier.
     * @param[in] id        Transverse identifier.
     * @param[in] threshold Transverse threshold.
     */
    void setTransverseThreshold(const int id, const double threshold)
    {
      m_TransverseThreshold.insert(std::pair<int, double>(id, threshold));
    }

    /**
     * Set the transverse probability density function (analytical): horizontal scale for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] scaleX Horizontal scale.
     */
    void setTransverseScaleX(const int id, const double scaleX)
    {
      m_TransverseScaleX.insert(std::pair<int, double>(id, scaleX));
    }

    /**
     * Set the transverse probability density function (analytical): vertical scale for a given identifier.
     * @param[in] id     Transverse identifier.
     * @param[in] scaleY Vertical scale.
     */
    void setTransverseScaleY(const int id, const double scaleY)
    {
      m_TransverseScaleY.insert(std::pair<int, double>(id, scaleY));
    }

    /**
     * Get the longitudinal probability density function for a given identifier.
     * @param[in] id Longitudinal identifier.
     */
    const std::vector<double>& getLongitudinalPDF(const int id) const;

    /**
     * Get the transverse probability density function for a given identifier.
     * @param[in] id Transverse identifier.
     */
    const std::vector<double>& getTransversePDF(const int id) const;

    /**
     * Get the transverse probability density function (analytical): threshold for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseThreshold(const int id) const;

    /**
     * Get the transverse probability density function (analytical): horizontal scale for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseScaleX(const int id) const;

    /**
     * Get the transverse probability density function (analytical): vertical scale for a given identifier.
     * @param[in] id Transverse identifier.
     */
    double getTransverseScaleY(const int id) const;

    /** Longitudinal probability density function. */
    std::map<int, std::vector<double>> m_LongitudinalPDF;

    /** Transverse probability density function. */
    std::map<int, std::vector<double>> m_TransversePDF;

    /** Transverse probability density function (analytical): threshold. */
    std::map<int, double> m_TransverseThreshold;

    /** Transverse probability density function (analytical): horizontal scale. */
    std::map<int, double> m_TransverseScaleX;

    /** Transverse probability density function (analytical): vertical scale. */
    std::map<int, double> m_TransverseScaleY;

    /** Class version. */
    ClassDef(KLMLikelihoodParameters, 1);

  };

}
