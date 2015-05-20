/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef IFNOTCINT_H_
#define IFNOTCINT_H_

#ifdef __CINT__
#define IF_NOT_CINT(x)
#else
#define IF_NOT_CINT(x) x
#endif

#endif
