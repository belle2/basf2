#pragma once

#include <tracking/dqmUtils/DQMHistoModuleBase.h>

namespace Belle2 {

  template <typename AType>
  class Parameter {
  public:
    Parameter(AType defaultValue)
    {
      m_value = defaultValue;
      m_isSet = false;
      m_isSetOneTime = false;
    }
    void Set(AType value);
    void Set(Parameter<AType> parameter);
    void SetOneTime(AType value);
    void SetOneTime(Parameter<AType> parameter);
    AType Get();

  private:
    AType m_value;
    AType m_oneTimeValue;
    bool m_isSet;
    bool m_isSetOneTime;
  };

  template <class AType>
  void Parameter<AType>::Set(AType value)
  {
    m_value = value;
    m_isSet = true;
  }

  template <class AType>
  void Parameter<AType>::Set(Parameter<AType> parameter)
  {
    if (parameter.m_isSet)
      Set(parameter.m_value);
  }

  template <class AType>
  void Parameter<AType>::SetOneTime(AType value)
  {
    m_oneTimeValue = value;
    m_isSetOneTime = true;
  }

  template <class AType>
  void Parameter<AType>::SetOneTime(Parameter<AType> parameter)
  {
    if (parameter.m_isSet)
      SetOneTime(parameter.m_value);
  }

  template <class AType>
  AType Parameter<AType>::Get()
  {
    if (m_isSetOneTime) {
      m_isSetOneTime = false;
      return m_oneTimeValue;
    } else {
      return m_value;
    }
  }

  class THFAxis {
  public:
    THFAxis();

    THFAxis(int nbins, double low, double up, const char* title)
    {
      m_nbins.Set(nbins);
      m_low.Set(low);
      m_up.Set(up);
      m_title.Set(title);
    }

    THFAxis(THFAxis& axis)
    {
      m_nbins.Set(axis.m_nbins);
      m_low.Set(axis.m_low);
      m_up.Set(axis.m_up);
      m_title.Set(axis.m_title);
    }

    THFAxis& nbins(int nbins) { m_nbins.Set(nbins); return *this; }
    THFAxis& low(double low) { m_low.Set(low); return *this; }
    THFAxis& up(double up) { m_up.Set(up); return *this; }
    THFAxis& title(const char* title) { m_title.Set(title); return *this; }

  private:
    Parameter<int> m_nbins = Parameter(0);
    Parameter<double> m_low = Parameter(.0);
    Parameter<double> m_up = Parameter(.0);
    Parameter<const char*> m_title = Parameter("");

    friend class THFFactory;
  };

  class THFFactory {
  public:
    THFFactory(DQMHistoModuleBase* histoModule)
    {
      m_histoModule = histoModule;
    }

    THFFactory& xAxis(THFAxis& axis)
    {
      m_nbinsx.SetOneTime(axis.m_nbins);
      m_xlow.SetOneTime(axis.m_low);
      m_xup.SetOneTime(axis.m_up);
      m_xTitle.SetOneTime(axis.m_title);

      return *this;
    }

    THFFactory& yAxis(THFAxis& axis)
    {
      m_nbinsy.SetOneTime(axis.m_nbins);
      m_ylow.SetOneTime(axis.m_low);
      m_yup.SetOneTime(axis.m_up);
      m_yTitle.SetOneTime(axis.m_title);

      return *this;
    }

    THFFactory& xAxisSet(THFAxis& axis)
    {
      m_nbinsx.Set(axis.m_nbins);
      m_xlow.Set(axis.m_low);
      m_xup.Set(axis.m_up);
      m_xTitle.Set(axis.m_title);

      return *this;
    }

    THFFactory& yAxisSet(THFAxis& axis)
    {
      m_nbinsy.Set(axis.m_nbins);
      m_ylow.Set(axis.m_low);
      m_yup.Set(axis.m_up);
      m_yTitle.Set(axis.m_title);

      return *this;
    }

    TH1F* CreateTH1F(const char* name, const char* title);
    TH2F* CreateTH2F(const char* name, const char* title);

    TH1F** CreateLayersTH1F(boost::format nameTemplate, boost::format titleTemplate);
    TH2F** CreateLayersTH2F(boost::format nameTemplate, boost::format titleTemplate);
    TH1F** CreateSensorsTH1F(boost::format nameTemplate, boost::format titleTemplate);
    TH2F** CreateSensorsTH2F(boost::format nameTemplate, boost::format titleTemplate);

    THFFactory& nbinsxSet(int nbinsx) { m_nbinsx.Set(nbinsx); return *this; }
    THFFactory& xlowSet(double xlow) { m_xlow.Set(xlow); return *this; }
    THFFactory& xupSet(double xup) { m_xup.Set(xup); return *this; }
    THFFactory& nbinsySet(int nbinsy) { m_nbinsy.Set(nbinsy); return *this; }
    THFFactory& ylowSet(double ylow) { m_ylow.Set(ylow); return *this; }
    THFFactory& yupSet(double yup) { m_yup.Set(yup); return *this; }
    THFFactory& xTitleSet(const char* xTitle) { m_xTitle.Set(xTitle); return *this; }
    THFFactory& yTitleSet(const char* yTitle) { m_yTitle.Set(yTitle); return *this; }
    THFFactory& zTitleSet(const char* zTitle) { m_zTitle.Set(zTitle); return *this; }

    THFFactory& nbinsx(int nbinsx) { m_nbinsx.SetOneTime(nbinsx); return *this; }
    THFFactory& xlow(double xlow) { m_xlow.SetOneTime(xlow); return *this; }
    THFFactory& xup(double xup) { m_xup.SetOneTime(xup); return *this; }
    THFFactory& nbinsy(int nbinsy) { m_nbinsy.SetOneTime(nbinsy); return *this; }
    THFFactory& ylow(double ylow) { m_ylow.SetOneTime(ylow); return *this; }
    THFFactory& yup(double yup) { m_yup.SetOneTime(yup); return *this; }
    THFFactory& xTitle(const char* xTitle) { m_xTitle.SetOneTime(xTitle); return *this; }
    THFFactory& yTitle(const char* yTitle) { m_yTitle.SetOneTime(yTitle); return *this; }
    THFFactory& zTitle(const char* zTitle) { m_zTitle.SetOneTime(zTitle); return *this; }

  private:
    DQMHistoModuleBase* m_histoModule;

    Parameter<int> m_nbinsx = Parameter(0);
    Parameter<double> m_xlow = Parameter(.0);
    Parameter<double> m_xup = Parameter(.0);
    Parameter<const char*> m_xTitle = Parameter("");
    Parameter<const char*> m_yTitle = Parameter("");
    Parameter<int> m_nbinsy = Parameter(0);
    Parameter<double> m_ylow = Parameter(.0);
    Parameter<double> m_yup = Parameter(.0);
    Parameter<const char*> m_zTitle = Parameter("");
  };
}