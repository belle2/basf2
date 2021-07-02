/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/datastore/RelationsObject.h>

#include <framework/utilities/HTML.h>

using namespace Belle2;

std::string _RelationsInterfaceImpl::htmlToPlainText(const std::string& html)
{
  return HTML::htmlToPlainText(html);
}
