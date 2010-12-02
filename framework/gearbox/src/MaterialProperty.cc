/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/MaterialProperty.h>

using namespace std;
using namespace Belle2;


MaterialProperty::MaterialProperty() : m_name("")
{
  m_valueTree.Branch("energy", &m_energy, "E/D");
  m_valueTree.Branch("value", &m_value, "V/D");
}


MaterialProperty::MaterialProperty(const std::string& name) : m_name(name)
{
  m_valueTree.Branch("energy", &m_energy, "E/D");
  m_valueTree.Branch("value", &m_value, "V/D");
}

void MaterialProperty::addValue(double energy, double value)
{
  m_energy = energy;
  m_value = value;
  m_valueTree.Fill();
}


ClassImp(MaterialProperty)
