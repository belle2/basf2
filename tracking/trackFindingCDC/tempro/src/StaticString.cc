/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/StaticString.h"

using namespace Belle2;
using namespace TrackFindingCDC;

const char StaticString<>::chars[1] = {'\0' };
const char StaticString<>::first = '\0';
const char* StaticString<>::c_str() { return &(chars[0]); }
