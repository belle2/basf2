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


void FlavorTaggerInfoMap::setTargetTrackLevel(const std::string& category, const Track* track)
{
  m_targetTrackLevel.insert(std::pair<std::string, const Track*>(category, track));
}


void FlavorTaggerInfoMap::setProbTrackLevel(const std::string& category, float probability)
{
  m_probTrackLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTaggerInfoMap::setTargetEventLevel(const std::string& category, const Belle2::Track* track)
{
  m_targetEventLevel.insert(std::pair<std::string, const Track*>(category, track));
}


void FlavorTaggerInfoMap::setProbEventLevel(const std::string& category, float probability)
{
  m_probEventLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTaggerInfoMap::setQpCategory(const std::string& category, float qp)
{
  m_qpCategory.insert(std::pair<std::string, float>(category, qp));
}

void FlavorTaggerInfoMap::setHasTrueTarget(const std::string& category, float isTrue)
{
  m_hasTrueTarget.insert(std::pair<std::string, float>(category, isTrue));
}

void FlavorTaggerInfoMap::setIsTrueCategory(const std::string& category, float isTrue)
{
  m_isTrueCategory.insert(std::pair<std::string, float>(category, isTrue));
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


std::map<std::string, const Belle2::Track*> FlavorTaggerInfoMap::getTargetTrackLevel() const
{
  return m_targetTrackLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getProbTrackLevel() const
{
  return m_probTrackLevel;
}

std::map<std::string, const Belle2::Track*> FlavorTaggerInfoMap::getTargetEventLevel() const
{
  return m_targetEventLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getProbEventLevel() const
{
  return m_probEventLevel;
}

std::map<std::string, float> FlavorTaggerInfoMap::getQpCategory() const
{
  return m_qpCategory;
}

std::map<std::string, float> FlavorTaggerInfoMap::getHasTrueTarget() const
{
  return m_hasTrueTarget;
}

std::map<std::string, float> FlavorTaggerInfoMap::getIsTrueCategory() const
{
  return m_isTrueCategory;
}

float FlavorTaggerInfoMap::getQrCombined() const
{
  return m_qrCombined;
}

float FlavorTaggerInfoMap::getB0Probability() const
{
  return m_B0Probability;
}

float FlavorTaggerInfoMap::getB0barProbability() const
{
  return m_B0barProbability;
}
