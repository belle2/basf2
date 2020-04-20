#pragma once

#include <tracking/dqmUtils/BaseDQMHistogramModule.h>

namespace Belle2 {

  template <typename T>
  class Parameter {
  public:
    Parameter(T defaultValue)
    {
      _value = defaultValue;
      isSet = false;
      isSetOneTime = false;
    }
    void Set(T value);
    void Set(Parameter<T> parameter);
    void SetOneTime(T value);
    void SetOneTime(Parameter<T> parameter);
    T Get();

  private:
    T _value;
    T _oneTimeValue;
    bool isSet;
    bool isSetOneTime;
  };

  template <class T>
  void Parameter<T>::Set(T value)
  {
    _value = value;
    isSet = true;
  }

  template <class T>
  void Parameter<T>::Set(Parameter<T> parameter)
  {
    if (parameter.isSet)
      Set(parameter._value);
  }

  template <class T>
  void Parameter<T>::SetOneTime(T value)
  {
    _oneTimeValue = value;
    isSetOneTime = true;
  }

  template <class T>
  void Parameter<T>::SetOneTime(Parameter<T> parameter)
  {
    if (parameter.isSet)
      SetOneTime(parameter._value);
  }

  template <class T>
  T Parameter<T>::Get()
  {
    if (isSetOneTime) {
      isSetOneTime = false;
      return _oneTimeValue;
    } else {
      return _value;
    }
  }

  class THFAxis {
  public:
    THFAxis();

    THFAxis(int nbins, double low, double up, const char* title)
    {
      _nbins.Set(nbins);
      _low.Set(low);
      _up.Set(up);
      _title.Set(title);
    }

    THFAxis(THFAxis& axis)
    {
      _nbins.Set(axis._nbins);
      _low.Set(axis._low);
      _up.Set(axis._up);
      _title.Set(axis._title);
    }

    THFAxis& nbins(int nbins) { _nbins.Set(nbins); return *this; }
    THFAxis& low(double low) { _low.Set(low); return *this; }
    THFAxis& up(double up) { _up.Set(up); return *this; }
    THFAxis& title(const char* title) { _title.Set(title); return *this; }

  private:
    Parameter<int> _nbins = Parameter(0);
    Parameter<double> _low = Parameter(.0);
    Parameter<double> _up = Parameter(.0);
    Parameter<const char*> _title = Parameter("");

    friend class THFFactory;
  };

  class THFFactory {
  public:
    THFFactory(BaseDQMHistogramModule* set);

    THFFactory& xAxis(THFAxis& axis)
    {
      _nbinsx.SetOneTime(axis._nbins);
      _xlow.SetOneTime(axis._low);
      _xup.SetOneTime(axis._up);
      _xTitle.SetOneTime(axis._title);

      return *this;
    }

    THFFactory& yAxis(THFAxis& axis)
    {
      _nbinsy.SetOneTime(axis._nbins);
      _ylow.SetOneTime(axis._low);
      _yup.SetOneTime(axis._up);
      _yTitle.SetOneTime(axis._title);

      return *this;
    }

    THFFactory& xAxisSet(THFAxis& axis)
    {
      _nbinsx.Set(axis._nbins);
      _xlow.Set(axis._low);
      _xup.Set(axis._up);
      _xTitle.Set(axis._title);

      return *this;
    }

    THFFactory& yAxisSet(THFAxis& axis)
    {
      _nbinsy.Set(axis._nbins);
      _ylow.Set(axis._low);
      _yup.Set(axis._up);
      _yTitle.Set(axis._title);

      return *this;
    }

    TH1F* CreateTH1F(const char* name, const char* title);
    TH2F* CreateTH2F(const char* name, const char* title);

    TH1F** CreateLayersTH1F(boost::format nameTemplate, boost::format titleTemplate);
    TH2F** CreateLayersTH2F(boost::format nameTemplate, boost::format titleTemplate);
    TH1F** CreateSensorsTH1F(boost::format nameTemplate, boost::format titleTemplate);
    TH2F** CreateSensorsTH2F(boost::format nameTemplate, boost::format titleTemplate);

    THFFactory& nbinsxSet(int nbinsx) { _nbinsx.Set(nbinsx); return *this; }
    THFFactory& xlowSet(double xlow) { _xlow.Set(xlow); return *this; }
    THFFactory& xupSet(double xup) { _xup.Set(xup); return *this; }
    THFFactory& nbinsySet(int nbinsy) { _nbinsy.Set(nbinsy); return *this; }
    THFFactory& ylowSet(double ylow) { _ylow.Set(ylow); return *this; }
    THFFactory& yupSet(double yup) { _yup.Set(yup); return *this; }
    THFFactory& xTitleSet(const char* xTitle) { _xTitle.Set(xTitle); return *this; }
    THFFactory& yTitleSet(const char* yTitle) { _yTitle.Set(yTitle); return *this; }
    THFFactory& zTitleSet(const char* zTitle) { _zTitle.Set(zTitle); return *this; }

    THFFactory& nbinsx(int nbinsx) { _nbinsx.SetOneTime(nbinsx); return *this; }
    THFFactory& xlow(double xlow) { _xlow.SetOneTime(xlow); return *this; }
    THFFactory& xup(double xup) { _xup.SetOneTime(xup); return *this; }
    THFFactory& nbinsy(int nbinsy) { _nbinsy.SetOneTime(nbinsy); return *this; }
    THFFactory& ylow(double ylow) { _ylow.SetOneTime(ylow); return *this; }
    THFFactory& yup(double yup) { _yup.SetOneTime(yup); return *this; }
    THFFactory& xTitle(const char* xTitle) { _xTitle.SetOneTime(xTitle); return *this; }
    THFFactory& yTitle(const char* yTitle) { _yTitle.SetOneTime(yTitle); return *this; }
    THFFactory& zTitle(const char* zTitle) { _zTitle.SetOneTime(zTitle); return *this; }

  private:
    BaseDQMHistogramModule* histogramSet;

    Parameter<int> _nbinsx = Parameter(0);
    Parameter<double> _xlow = Parameter(.0);
    Parameter<double> _xup = Parameter(.0);
    Parameter<const char*> _xTitle = Parameter("");
    Parameter<const char*> _yTitle = Parameter("");
    Parameter<int> _nbinsy = Parameter(0);
    Parameter<double> _ylow = Parameter(.0);
    Parameter<double> _yup = Parameter(.0);
    Parameter<const char*> _zTitle = Parameter("");
  };
}