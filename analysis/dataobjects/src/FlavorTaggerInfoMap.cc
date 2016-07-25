/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/FlavorTaggerInfoMap.h>

using namespace Belle2;



/************************SETTERS***********************/


void FlavorTaggerInfoMap::setTargetTrackLevel(std::string category, const Track* track)
{
  m_targetTrackLevel.insert(std::pair<std::string, const Track*>(category, track));
}


void FlavorTaggerInfoMap::setProbTrackLevel(std::string category, float probability)
{
  m_probTrackLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTaggerInfoMap::setTargetEventLevel(std::string category, const Belle2::Track* track)
{
  m_targetEventLevel.insert(std::pair<std::string, const Track*>(category, track));
}


void FlavorTaggerInfoMap::setProbEventLevel(std::string category, float probability)
{
  m_probEventLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTaggerInfoMap::setQrCategory(std::string category, float qr)
{
  m_qrCategory.insert(std::pair<std::string, float>(category, qr));
}

void FlavorTaggerInfoMap::setQrCombined(float qr)
{
  m_qrCombined = qr;
}


void FlavorTaggerInfoMap::setB0Probability(float B0Probability)
{
  m_B0Probability = B0Probability;
}


void FlavorTaggerInfoMap::setB0barProbability(float B0barProbability)
{
  m_B0barProbability = B0barProbability;
}


/************************GETTERS***********************/


std::map<std::string, const Belle2::Track*> FlavorTaggerInfoMap::getTargetTrackLevel()
{
  return m_targetTrackLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getProbTrackLevel()
{
  return m_probTrackLevel;
}

std::map<std::string, const Belle2::Track*> FlavorTaggerInfoMap::getTargetEventLevel()
{
  return m_targetEventLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getProbEventLevel()
{
  return m_probEventLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getQrCategory()
{
  return m_qrCategory;
}

float FlavorTaggerInfoMap::getQrCombined()
{
  return m_qrCombined;
}

float FlavorTaggerInfoMap::getB0Probability()
{
  return m_B0Probability;
}

float FlavorTaggerInfoMap::getB0barProbability()
{
  return m_B0barProbability;
}
