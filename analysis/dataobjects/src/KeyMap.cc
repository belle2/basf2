/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/KeyMap.h>
#include <framework/logging/Logger.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>

using namespace Belle2;

int KeyMap::numberOfDimensions()
{
  if ((not m_1DkeyTable.second.empty()) and (m_2DkeyTable.second.empty()) and (m_3DkeyTable.second.empty())) {
    return 1;
  } else if ((m_1DkeyTable.second.empty()) and (not m_2DkeyTable.second.empty()) and (m_3DkeyTable.second.empty())) {
    return 2;
  } else if ((m_1DkeyTable.second.empty()) and (m_2DkeyTable.second.empty()) and (not m_3DkeyTable.second.empty())) {
    return 3;
  } else {
    B2ERROR("Key map is incorrectly intialized: dimension can't be defined.");
  }
}

void KeyMap::dimensionsCheck(int ndim)
{
  if (ndim != this->numberOfDimensions()) {
    B2ERROR("You checked if " << this->numberOfDimensions() << "D table is " << ndim << "D.");
  }
}

bool KeyMap::isKey(BinLimits var1_minimax)
{
  this->dimensionsCheck(1);
  if (m_1DkeyTable.second.find(var1_minimax) != m_1DkeyTable.second.end()) {
    return true;
  }
  return false;
}

bool KeyMap::isKey(BinLimits var1_minimax, BinLimits var2_minimax)
{
  this->dimensionsCheck(2);
  if (m_2DkeyTable.second.find(var1_minimax) != m_2DkeyTable.second.end()) {
    if (m_2DkeyTable.second.find(var1_minimax)->second.find(var2_minimax) != m_2DkeyTable.second.find(var1_minimax)->second.end()) {
      return true;
    }
  }
  return false;
}

bool KeyMap::isKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax)
{
  this->dimensionsCheck(3);
  if (m_3DkeyTable.second.find(var1_minimax) != m_3DkeyTable.second.end()) {
    if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax) != m_3DkeyTable.second.find(var1_minimax)->second.end()) {
      if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.find(var3_minimax) != m_3DkeyTable.second.find(
            var1_minimax)->second.find(var2_minimax)->second.end()) {
        return true;
      }
    }
    return false;
  }
}

bool KeyMap::isKey(double key)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      for (auto var1_minimax : m_1DkeyTable.second) {
        if (key ==  var1_minimax.second) {
          return true;
        }
      }
      return false;
      break;
    case 2:
      for (auto var1_minimax : m_2DkeyTable.second) {
        for (auto var2_minimax : var1_minimax.second) {
          if (key ==  var2_minimax.second) {
            return true;
          }
        }
      }
      return false;
      break;
    case 3:
      for (auto var1_minimax : m_3DkeyTable.second) {
        for (auto var2_minimax : var1_minimax.second) {
          for (auto var3_minimax : var2_minimax.second) {
            if (key ==  var3_minimax.second) {
              return true;
            }
          }
        }
      }
      return false;
      break;
    default:
      B2ERROR("Error in finding of the key due to dumensionality of the table");
      return false;
  }
}

double KeyMap::getKey(BinLimits var1_minimax)
{
  this->dimensionsCheck(1);
  if (not this->isKey(var1_minimax)) {
    B2ERROR("Trying to get unexisting kinematic key: \n var1 in [" << var1_minimax.first << "; " << var1_minimax.second <<
            "].");
  }
  return m_1DkeyTable.second.find(var1_minimax)->second;
}

double KeyMap::getKey(BinLimits var1_minimax, BinLimits var2_minimax)
{
  this->dimensionsCheck(2);
  if (not this->isKey(var1_minimax, var2_minimax)) {
    B2ERROR("Trying to get unexisting kinematic key: \n var1 in [" << var1_minimax.first << "; " << var1_minimax.second <<
            "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second << "].");
  }
  return m_2DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second;
}

double KeyMap::getKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax)
{
  this->dimensionsCheck(3);
  if (not this->isKey(var1_minimax, var2_minimax, var3_minimax)) {
    B2ERROR("Trying to get unexisting kinematic key: \n var1 in [" << var1_minimax.first << "; " << var1_minimax.second <<
            "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second <<
            "], \n var3 in [" << var3_minimax.first << "; " << var3_minimax.second << "].");
  }
  return m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.find(var3_minimax)->second;
}

// Add new kinematic key
double KeyMap::addKey(BinLimits var1_minimax)
{
  this->dimensionsCheck(1);
  if (this->isKey(var1_minimax)) {
    return this->getKey(var1_minimax);
  }
  double bin_number = m_1DkeyTable.second.size();
  while (this->isKey(bin_number)) {
    bin_number += 1;
  }
  return this->addKey(var1_minimax, bin_number);
}

double KeyMap::addKey(BinLimits var1_minimax, BinLimits var2_minimax)
{
  this->dimensionsCheck(2);
  if (this->isKey(var1_minimax, var2_minimax)) {
    return this->getKey(var1_minimax, var2_minimax);
  }
  double bin_number = m_2DkeyTable.second.size();
  while (this->isKey(bin_number)) {
    bin_number += 1;
  }
  return this->addKey(var1_minimax, var2_minimax, bin_number);
}

double KeyMap::addKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax)
{
  this->dimensionsCheck(3);
  if (this->isKey(var1_minimax, var2_minimax, var3_minimax)) {
    return this->getKey(var1_minimax, var2_minimax, var3_minimax);
  }
  double bin_number = m_3DkeyTable.second.size();
  while (this->isKey(bin_number)) {
    bin_number += 1;
  }
  return this->addKey(var1_minimax, var2_minimax, var3_minimax, bin_number);
}

// Add entry to kinematic key table
double KeyMap::addKey(BinLimits var1_minimax, double key_ID)
{
  this->dimensionsCheck(1);
  if (this->isKey(var1_minimax)) {
    double found_key = this->getKey(var1_minimax);
    if (found_key == key_ID) {
      return key_ID;
    } else {
      B2WARNING("Trying to assign existing limits to the new kinematic key # " << key_ID << ": \n var1 in [" << var1_minimax.first << "; "
                << var1_minimax.second <<
                "]\n. Ignoring the attempt.");
      return found_key;
    }
  }


  m_1DkeyTable.second.insert(std::make_pair(var1_minimax, key_ID));

  return key_ID;
}

double KeyMap::addKey(BinLimits var1_minimax, BinLimits var2_minimax,
                      double key_ID)
{
  this->dimensionsCheck(2);
  if (this->isKey(var1_minimax, var2_minimax)) {
    double found_key = this->getKey(var1_minimax, var2_minimax);
    if (found_key == key_ID) {
      return key_ID;
    } else {
      B2WARNING("Trying to assign existing limits to the new kinematic key # " << key_ID << ": \n var1 in [" << var1_minimax.first << "; "
                << var1_minimax.second <<
                "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second << "].\n. Ignoring the attempt.");
      return found_key;
    }
  }

  if (m_2DkeyTable.second.find(var1_minimax) != m_2DkeyTable.second.end()) {
    m_2DkeyTable.second.find(var1_minimax)->second.insert(std::make_pair(var2_minimax, key_ID));
  } else {
    Nameless1DMap var2_key_map;
    var2_key_map[var2_minimax] = key_ID;
    m_2DkeyTable.second[var1_minimax] = var2_key_map;
  }
  return key_ID;
}

double KeyMap::addKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax,
                      double key_ID)
{
  this->dimensionsCheck(3);
  if (this->isKey(var1_minimax, var2_minimax, var3_minimax)) {
    double found_key = this->getKey(var1_minimax, var2_minimax, var3_minimax);
    if (found_key == key_ID) {
      return key_ID;
    } else {
      B2WARNING("Trying to assign existing limits to the new kinematic key # " << key_ID << ": \n var1 in [" << var1_minimax.first << "; "
                << var1_minimax.second <<
                "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second << "], \n var3 in ["
                << var3_minimax.first << "; " << var3_minimax.second << "].\n. Ignoring the attempt.");
      return found_key;
    }
  }

  if (m_3DkeyTable.second.find(var1_minimax) != m_3DkeyTable.second.end()) {
    if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax) != m_3DkeyTable.second.find(var1_minimax)->second.end()) {
      m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.insert(std::make_pair(var3_minimax, key_ID));
    } else {
      Nameless1DMap var3_key_map;
      var3_key_map[var3_minimax] = key_ID;
      m_3DkeyTable.second[var1_minimax][var2_minimax] = var3_key_map;
    }
  } else {
    Nameless1DMap var3_key_map;
    var3_key_map[var3_minimax] = key_ID;
    Nameless2DMap var2_key_map;
    var2_key_map[var2_minimax] = var3_key_map;
    m_3DkeyTable.second[var1_minimax] = var2_key_map;
  }
  return key_ID;
}



// Get kinematic key for the particle's momentum
double KeyMap::getKey(const Particle* p)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1: {
      double var1_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_1DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
      } else {
        var1_val = var1->function(p);
      }
      for (auto var1_minimax : m_1DkeyTable.second) {
        if (var1_minimax.first.first <= var1_val && var1_minimax.first.second > var1_val) {
          return var1_minimax.second;
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    case 2: {
      double var1_val;
      double var2_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_2DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(variables[1]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
      } else {
        var1_val = var1->function(p);
      }
      if (!var2) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
      } else {
        var2_val = var2->function(p);
      }
      for (auto var1_minimax : m_2DkeyTable.second) {
        if (var1_minimax.first.first <= var2_val && var1_minimax.first.second > var2_val) {
          for (auto var2_minimax : var1_minimax.second) {
            if (var2_minimax.first.first <= var1_val && var2_minimax.first.second > var1_val) {
              return var2_minimax.second;
            }
          }
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    case 3: {
      double var1_val;
      double var2_val;
      double var3_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_3DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(variables[1]);
      const Variable::Manager::Var* var3 = Variable::Manager::Instance().getVariable(variables[2]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
      } else {
        var1_val = var1->function(p);
      }
      if (!var2) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
      } else {
        var2_val = var2->function(p);
      }
      if (!var3) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
      } else {
        var3_val = var3->function(p);
      }
      for (auto var1_minimax : m_3DkeyTable.second) {
        if (var1_minimax.first.first <= var1_val && var1_minimax.first.second > var1_val) {
          for (auto var2_minimax : var1_minimax.second) {
            if (var2_minimax.first.first <= var2_val && var2_minimax.first.second > var2_val) {
              for (auto var3_minimax : var2_minimax.second) {
                if (var3_minimax.first.first <= var3_val && var3_minimax.first.second > var3_val) {
                  return var3_minimax.second;
                }
              }
            }
          }
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    default:
      B2ERROR("Error in finding of the key due to dimensionality of the table");
  }
  return -1;
}


std::string KeyMap::getVar1()
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      return m_1DkeyTable.first[0];
      break;
    case 2:
      return m_2DkeyTable.first[0];
      break;
    case 3:
      return m_3DkeyTable.first[0];
      break;
    default:
      B2ERROR("Error in gtting variable name due to dimensionality of the table");
  }
}

std::string KeyMap::getVar2()
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      B2ERROR("Trying to get name of the second variable from 1D table.");
      break;
    case 2:
      return m_2DkeyTable.first[1];
      break;
    case 3:
      return m_3DkeyTable.first[1];
      break;
    default:
      B2ERROR("Error in gtting variable name due to dimensionality of the table");
  }
}

std::string KeyMap::getVar3()
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      B2ERROR("Trying to get name of the third variable from 1D table.");
      break;
    case 2:
      B2ERROR("Trying to get name of the third variable from 2D table.");
      break;
    case 3:
      return m_3DkeyTable.first[2];
      break;
    default:
      B2ERROR("Error in gtting variable name due to dimensionality of the table");
  }

}

void KeyMap::setVar1(std::string name)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      m_1DkeyTable.first[0] = name;
      break;
    case 2:
      m_2DkeyTable.first[0] = name;
      break;
    case 3:
      m_3DkeyTable.first[0] = name;
      break;
    default:
      B2ERROR("Error in setting variable name due to dimensionality of the table");
  }
}

void KeyMap::setVar2(std::string name)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      B2ERROR("Trying to set name of the second variable for 1D table.");
      break;
    case 2:
      m_2DkeyTable.first[1] = name;
      break;
    case 3:
      m_3DkeyTable.first[1] = name;
      break;
    default:
      B2ERROR("Error in setting variable name due to dimensionality of the table");
  }
}

void KeyMap::setVar3(std::string name)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1:
      B2ERROR("Trying to set name of the third variable for 1D table.");
      break;
    case 2:
      B2ERROR("Trying to set name of the third variable for 2D table.");
      break;
    case 3:
      m_3DkeyTable.first[2] = name;
      break;
    default:
      B2ERROR("Error in setting variable name due to dimensionality of the table");
  }
}

std::vector<std::string> KeyMap::addNames(NDBin bin)
{
  std::vector<std::string> names;
  for (auto const& name : bin) {
    names.push_back(name.first);
  }
  /** Sorting names to avoid confusion */
  std::sort(names.begin(), names.end());
  int nDim = names.size();
  if (this->getVar1().empty()) {
    this->setVar1(names[0]);
  } else {
    if (this->getVar1() != names[0]) {
      B2ERROR("Attempt of change of the first variable name");
    }
  }
  if (nDim > 1) {
    if (this->getVar2().empty()) {
      this->setVar2(names[1]);
    } else {
      if (this->getVar2() != names[1]) {
        B2ERROR("Attempt of change of the second variable name");
      }
    }
  }

  if (nDim > 2) {
    if (this->getVar3().empty()) {
      this->setVar3(names[2]);
    } else {
      if (this->getVar3() != names[2]) {
        B2ERROR("Attempt of change of the third variable name");
      }
    }
  }
  return names;
}

double KeyMap::addKey(NDBin bin, double key_ID)
{
  std::vector<std::string>  names = this->addNames(bin);
  int nDim = names.size();
  switch (nDim) {
    case 1:
      return this->addKey(bin[names[0]], key_ID);
      break;
    case 2:
      return this->addKey(bin[names[0]], bin[names[1]], key_ID);
      break;
    case 3:
      return this->addKey(bin[names[0]], bin[names[1]], bin[names[2]], key_ID);
      break;
    default:
      B2ERROR("Error in adding key due to dimensionality of the table");
  }
}

double KeyMap::addKey(NDBin bin)
{
  std::vector<std::string> names = this->addNames(bin);
  int nDim = names.size();
  switch (nDim) {
    case 1:
      return this->addKey(bin[names[0]]);
      break;
    case 2:
      return this->addKey(bin[names[0]], bin[names[1]]);
      break;
    case 3:
      return this->addKey(bin[names[0]], bin[names[1]], bin[names[2]]);
      break;
    default:
      B2ERROR("Error in adding key due to dimensionality of the table");
  }
}
