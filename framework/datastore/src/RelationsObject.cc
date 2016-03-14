/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/RelationsObject.h>

#include <framework/utilities/HTML.h>

using namespace Belle2;

std::string _RelationsInterfaceImpl::htmlToPlainText(const std::string& html)
{
  return HTML::htmlToPlainText(html);
}
