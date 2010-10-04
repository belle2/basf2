// File and Version Information:
//      $Header$
//
// Description:
// Author: Heather Kelly

#include <arich/geoarich/optical.h>
#include <framework/logging/Logger.h>
#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <string>
#include <vector>


// Allocate the TObjArrays just once
using namespace std;
using namespace boost;


Optical::Optical()
{
  cout << "hmmm" << endl;
}

Optical::~Optical()
{
  Clear();

}

vector<property>* Optical::get_properties()
{
  return &properties;
}

void Optical::set_property(string namee, vector<double> *energies, vector<double>  *values)
{
  property m_property;
  m_property.name = namee;
  m_property.energy = *energies;
  m_property.value = *values;
  properties.push_back(m_property);
}



