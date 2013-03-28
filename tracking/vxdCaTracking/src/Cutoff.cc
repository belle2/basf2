/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/ClusterInfo.h"

using namespace std;
using namespace Belle2;

void Cutoff::addQuantile(std::pair<std::string, double> newQuantile) { m_quantiles.push_back(newQuantile); }
