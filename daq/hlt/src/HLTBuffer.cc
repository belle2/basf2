/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "framework/dcore/HLTBuffer.h"

using namespace Belle2;

HLTBuffer::HLTBuffer()
{
  m_buffer.clear();
}

HLTBuffer::~HLTBuffer()
{
}

void HLTBuffer::put(const std::string data)
{
  m_buffer.push_back(data);
}

std::string HLTBuffer::get()
{
  std::string data = m_buffer.front();
  m_buffer.erase(m_buffer.begin());

  return data;
}

int HLTBuffer::size()
{
  return m_buffer.size();
}

