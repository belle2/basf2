/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/base/Exception.h"

using namespace Belle2;

Exception::Exception()
  : m_comment(""), m_err(0)
{
}

Exception::Exception(const std::string& comment)
  : m_comment(comment), m_err(0)
{
}

Exception::Exception(const std::string& comment,
                     int err)
  : m_comment(comment), m_err(err)
{
}

#if __GNUC__ >= 7
const char* Exception::what() const noexcept
#else
const char* Exception::what() const throw()
#endif
{
  return m_comment.c_str();
}
