/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jachym Bartik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dqmUtils/HistogramFactory.h>

using namespace Belle2::HistogramFactory;
using namespace std;

TH1F* Factory::CreateTH1F(string name, string title)
{
  return m_histoModule->Create(name, title, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_xTitle.Get(), m_yTitle.Get());
}

TH2F* Factory::CreateTH2F(string name, string title)
{
  return m_histoModule->Create(name, title, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_nbinsy.Get(), m_ylow.Get(), m_yup.Get(),
                               m_xTitle.Get(), m_yTitle.Get(), m_zTitle.Get());
}

TH1F** Factory::CreateLayersTH1F(boost::format nameTemplate, boost::format titleTemplate)
{
  return m_histoModule->CreateLayers(nameTemplate, titleTemplate, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_xTitle.Get(),
                                     m_yTitle.Get());
}

TH2F** Factory::CreateLayersTH2F(boost::format nameTemplate, boost::format titleTemplate)
{
  return m_histoModule->CreateLayers(nameTemplate, titleTemplate, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_nbinsy.Get(),
                                     m_ylow.Get(), m_yup.Get(), m_xTitle.Get(), m_yTitle.Get(), m_zTitle.Get());
}

TH1F** Factory::CreateSensorsTH1F(boost::format nameTemplate, boost::format titleTemplate)
{
  return m_histoModule->CreateSensors(nameTemplate, titleTemplate, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_xTitle.Get(),
                                      m_yTitle.Get());
}

TH2F** Factory::CreateSensorsTH2F(boost::format nameTemplate, boost::format titleTemplate)
{
  return m_histoModule->CreateSensors(nameTemplate, titleTemplate, m_nbinsx.Get(), m_xlow.Get(), m_xup.Get(), m_nbinsy.Get(),
                                      m_ylow.Get(), m_yup.Get(), m_xTitle.Get(), m_yTitle.Get(), m_zTitle.Get());
}