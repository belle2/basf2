/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

namespace Belle2::HistogramFactory {
  /**
   * This class represents a quantity which value can be set both permanently and temporarily.
   * Temporary value has priority but is invalidated when is read for the first time.
   * Quantity can be of any type.
   * The class used in Factory to save some lines of code.
   */
  template <typename AType>
  class Parameter {
  public:
    /** Constructor.
    * @param defaultValue - a value which is returned when permanent nor temporary value is set. */
    explicit Parameter(const AType& defaultValue) :
      m_value(defaultValue), m_temporaryValue(defaultValue), m_isSet(false), m_isSetTemporarily(false)
    {}

    /** Permanently sets inner quantity to given value.
    * @param value - a value we want to set */
    void Set(const AType& value);
    /** Copy permanent value from another parameter but only if its permanent value is set
    * @param parameter - another parameter whose permanent value is copied */
    void Set(const Parameter<AType>& parameter);
    /** Temporarily sets inner quantity to given value
    * @param value - a value we want to set */
    void SetTemporarily(const AType& value);
    /** Copy temporary value from another parameter but only if its temporary value is set
    * @param parameter - another parameter whose temporary value is copied */
    void SetTemporarily(const Parameter<AType>& parameter);

    /** Returns value of the inner quantity.
    * Order is: temporary value (if is set), permanent value (if is set), default value. Temporary value is invalidated. */
    AType Get();

  private:
    /** permanent value */
    AType m_value;
    /** temporary value */
    AType m_temporaryValue;
    /** determines if the permanent value is set */
    bool m_isSet;
    /** determines if the temporary value is set */
    bool m_isSetTemporarily;
  };

  /** Sets the permanent value.
  * @param value - a value we want to assign to the permanent value. */
  template <class AType>
  void Parameter<AType>::Set(const AType& value)
  {
    m_value = value;
    m_isSet = true;
  }

  /** Copy the permanent value setting.
  * @param parameter - another parameter which permanent value setting we want to copy. */
  template <class AType>
  void Parameter<AType>::Set(const Parameter<AType>& parameter)
  {
    if (parameter.m_isSet)
      Set(parameter.m_value);
  }

  /** Sets the temporary value.
  * @param value - a value we want to assign to the temporary value. */

  template <class AType>
  void Parameter<AType>::SetTemporarily(const AType& value)
  {
    m_temporaryValue = value;
    m_isSetTemporarily = true;
  }

  /** Copy the temporary value setting.
  * @param parameter - another parameter which temporary value setting we want to copy. */
  template <class AType>
  void Parameter<AType>::SetTemporarily(const Parameter<AType>& parameter)
  {
    if (parameter.m_isSet)
      SetTemporarily(parameter.m_value);
  }

  /** Get value and reset temporary setting.
  * @return value - a temporary value if it's set, then permanent value if it's set, then default value. */
  template <class AType>
  AType Parameter<AType>::Get()
  {
    if (m_isSetTemporarily) {
      m_isSetTemporarily = false;
      return m_temporaryValue;
    } else {
      return m_value;
    }
  }

  /**
  * This class unites some parameters for Factory which describe one axis of histogram. Those parameters are nbins, low, up and title. */
  class Axis {
  public:
    /** Constructor. */
    Axis() {}

    /** Constructor.
    * @param nbins - number of bins along the axis
    * @param low - lower boundary of axis range
    * @param up - upper boundary of axis range
    * @param title - title of the axis */
    Axis(int nbins, double low, double up, std::string title)
    {
      m_nbins.Set(nbins);
      m_low.Set(low);
      m_up.Set(up);
      m_title.Set(title);
    }

    /** Set value of nbins */
    Axis& nbins(int nbins) { m_nbins.Set(nbins); return *this; }
    /** Set value of low */
    Axis& low(double low) { m_low.Set(low); return *this; }
    /** Set value of up */
    Axis& up(double up) { m_up.Set(up); return *this; }
    /** Set value of title */
    Axis& title(std::string title) { m_title.Set(title); return *this; }

  private:
    /** number of bins in the axis */
    Parameter<int> m_nbins = Parameter(0);
    /** lower boundary of axis range */
    Parameter<double> m_low = Parameter(.0);
    /** upper boundary of axis range */
    Parameter<double> m_up = Parameter(.0);
    /** title of axis */
    Parameter<std::string> m_title = Parameter(std::string());

    friend class Factory;
  };

  /**
  * This class is used for creating TH1F and TH2F objects.
  * Its main advantage is that parameters can be set individually and also permanently so they can be reused again.
  *
  * This class uses named parameters idiom via temporarily set values.
  *
  * Most of the methods return this object so they can be chained consecutively. */
  class Factory {
  public:
    /** Constructor.
    * @param histoModule - pointer on histogram module is needed because this class actually doesn't create histograms by itself, but it calls functions on the module instead. */
    explicit Factory(DQMHistoModuleBase* histoModule)
    {
      m_histoModule = histoModule;
    }

    /** Temporarily copies parameters for x axis from given Axis. */
    Factory& xAxis(const Axis& axis)
    {
      m_nbinsx.SetTemporarily(axis.m_nbins);
      m_xlow.SetTemporarily(axis.m_low);
      m_xup.SetTemporarily(axis.m_up);
      m_xTitle.SetTemporarily(axis.m_title);

      return *this;
    }

    /** Temporarily copies parameters for y axis from given Axis. */
    Factory& yAxis(const Axis& axis)
    {
      m_nbinsy.SetTemporarily(axis.m_nbins);
      m_ylow.SetTemporarily(axis.m_low);
      m_yup.SetTemporarily(axis.m_up);
      m_yTitle.SetTemporarily(axis.m_title);

      return *this;
    }

    /** Permanently copies parameters for x axis from given Axis. */
    Factory& xAxisDefault(const Axis& axis)
    {
      m_nbinsx.Set(axis.m_nbins);
      m_xlow.Set(axis.m_low);
      m_xup.Set(axis.m_up);
      m_xTitle.Set(axis.m_title);

      return *this;
    }

    /** Permanently copies parameters for y axis from given Axis. */
    Factory& yAxisDefault(const Axis& axis)
    {
      m_nbinsy.Set(axis.m_nbins);
      m_ylow.Set(axis.m_low);
      m_yup.Set(axis.m_up);
      m_yTitle.Set(axis.m_title);

      return *this;
    }

    /** Create TH1F with given name and title.
    * All temporarily set parameters needed to create the TH1F become invalidated. This is common for all following Create- functions. */
    TH1F* CreateTH1F(std::string name, std::string title);
    /** Create TH2F with given name and title. */
    TH2F* CreateTH2F(std::string name, std::string title);

    /** Create TH1F array for layers from given name template and title template. */
    TH1F** CreateLayersTH1F(boost::format nameTemplate, boost::format titleTemplate);
    /** Create TH2F array for layers from given name template and title template. */
    TH2F** CreateLayersTH2F(boost::format nameTemplate, boost::format titleTemplate);
    /** Create TH1F array for sensors from given name template and title template. */
    TH1F** CreateSensorsTH1F(boost::format nameTemplate, boost::format titleTemplate);
    /** Create TH2F array for sensors from given name template and title template. */
    TH2F** CreateSensorsTH2F(boost::format nameTemplate, boost::format titleTemplate);

    /** @name -Default functions
     * All the following functions permanently set the value of given parameter.
     * They also return this instance so they can be chained. */
    /** @{ */
    Factory& nbinsxDefault(int nbinsx) {    m_nbinsx.Set(nbinsx); return *this; } /**< Sets nbinsx permanently. */
    Factory& xlowDefault(double xlow) {     m_xlow.Set(xlow);     return *this; } /**< Sets xlow permanently. */
    Factory& xupDefault(double xup) {       m_xup.Set(xup);       return *this; } /**< Sets xup permanently. */
    Factory& nbinsyDefault(int nbinsy) {    m_nbinsy.Set(nbinsy); return *this; } /**< Sets nbinsy permanently. */
    Factory& ylowDefault(double ylow) {     m_ylow.Set(ylow);     return *this; } /**< Sets ylow permanently. */
    Factory& yupDefault(double yup) {       m_yup.Set(yup);       return *this; } /**< Sets yup permanently. */
    Factory& xTitleDefault(std::string xTitle) { m_xTitle.Set(xTitle); return *this; } /**< Sets xTitle permanently. */
    Factory& yTitleDefault(std::string yTitle) { m_yTitle.Set(yTitle); return *this; } /**< Sets yTitle permanently. */
    Factory& zTitleDefault(std::string zTitle) { m_zTitle.Set(zTitle); return *this; } /**< Sets zTitle permanently. */
    /** @} */

    /** @name Named parameters
     * All the following functions temporarily set the value of given parameter. This means that its value is invalidated after its first use in the Create- functions.
     * They also return this instance so they can be chained. */
    /** @{ */
    Factory& nbinsx(int nbinsx) {    m_nbinsx.SetTemporarily(nbinsx); return *this; } /**< Sets nbinsx temporarily */
    Factory& xlow(double xlow) {     m_xlow.SetTemporarily(xlow);     return *this; } /**< Sets xlow temporarily */
    Factory& xup(double xup) {       m_xup.SetTemporarily(xup);       return *this; } /**< Sets xup temporarily */
    Factory& nbinsy(int nbinsy) {    m_nbinsy.SetTemporarily(nbinsy); return *this; } /**< Sets nbinsy temporarily */
    Factory& ylow(double ylow) {     m_ylow.SetTemporarily(ylow);     return *this; } /**< Sets ylow temporarily */
    Factory& yup(double yup) {       m_yup.SetTemporarily(yup);       return *this; } /**< Sets yup temporarily */
    Factory& xTitle(std::string xTitle) { m_xTitle.SetTemporarily(xTitle); return *this; } /**< Sets xTitle temporarily */
    Factory& yTitle(std::string yTitle) { m_yTitle.SetTemporarily(yTitle); return *this; } /**< Sets yTitle temporarily */
    Factory& zTitle(std::string zTitle) { m_zTitle.SetTemporarily(zTitle); return *this; } /**< Sets zTitle temporarily */
    /** @} */

  private:
    /** DQM histogram module on which the Create- functions are called to create histograms */
    DQMHistoModuleBase* m_histoModule;

    Parameter<int> m_nbinsx = Parameter(0); /**< number of bins along the x axis */
    Parameter<double> m_xlow = Parameter(.0); /**< lower boundary of x axis range */
    Parameter<double> m_xup = Parameter(.0); /**< upper boundary of x axis range */
    Parameter<std::string> m_xTitle = Parameter(std::string()); /**< title of the x axis */
    Parameter<std::string> m_yTitle = Parameter(std::string()); /**< title of the y axis */
    Parameter<int> m_nbinsy = Parameter(0); /**< number of bins along the y axis */
    Parameter<double> m_ylow = Parameter(.0); /**< lower boundary of y axis range */
    Parameter<double> m_yup = Parameter(.0); /**< upper boundary of y axis range */
    Parameter<std::string> m_zTitle = Parameter(std::string()); /**< title of the z axis */
  };
}
