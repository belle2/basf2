#include <tracking/dqmUtils/THFFactory.h>

using namespace Belle2;

THFFactory::THFFactory(BaseDQMHistogramModule* set)
{
  histogramSet = set;
}

TH1F* THFFactory::CreateTH1F(const char* name, const char* title)
{
  return histogramSet->Create(name, title, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _xTitle.Get(), _yTitle.Get());
}

TH2F* THFFactory::CreateTH2F(const char* name, const char* title)
{
  return histogramSet->Create(name, title, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _nbinsy.Get(), _ylow.Get(), _yup.Get(),
                              _xTitle.Get(), _yTitle.Get(), _zTitle.Get());
}

TH1F** THFFactory::CreateLayersTH1F(boost::format nameTemplate, boost::format titleTemplate)
{
  return histogramSet->CreateLayers(nameTemplate, titleTemplate, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _xTitle.Get(),
                                    _yTitle.Get());
}

TH2F** THFFactory::CreateLayersTH2F(boost::format nameTemplate, boost::format titleTemplate)
{
  return histogramSet->CreateLayers(nameTemplate, titleTemplate, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _nbinsy.Get(), _ylow.Get(),
                                    _yup.Get(), _xTitle.Get(), _yTitle.Get(), _zTitle.Get());
}

TH1F** THFFactory::CreateSensorsTH1F(boost::format nameTemplate, boost::format titleTemplate)
{
  return histogramSet->CreateSensors(nameTemplate, titleTemplate, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _xTitle.Get(),
                                     _yTitle.Get());
}

TH2F** THFFactory::CreateSensorsTH2F(boost::format nameTemplate, boost::format titleTemplate)
{
  return histogramSet->CreateSensors(nameTemplate, titleTemplate, _nbinsx.Get(), _xlow.Get(), _xup.Get(), _nbinsy.Get(), _ylow.Get(),
                                     _yup.Get(), _xTitle.Get(), _yTitle.Get(), _zTitle.Get());
}