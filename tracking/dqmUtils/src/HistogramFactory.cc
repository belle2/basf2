/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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