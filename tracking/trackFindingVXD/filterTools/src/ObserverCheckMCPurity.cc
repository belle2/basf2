/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/trackFindingVXD/filterTools/ObserverCheckMCPurity.h>

using namespace Belle2;


/** defining member s_results of the ObserverCheckMCPurity. */
std::map<std::string, double*> ObserverCheckMCPurity::s_results;

/** defining member s_results of the ObserverCheckMCPurity. */
std::map<std::string, bool*> ObserverCheckMCPurity::s_wasAccepted;

/** defining member s_results of the ObserverCheckMCPurity. */
std::map<std::string, bool*> ObserverCheckMCPurity::s_wasUsed;

/** a ttree to store all the collected data. */
TTree* ObserverCheckMCPurity::s_ttree = nullptr;

/** stores the outer hit of a two-hit-combination. */
SpacePoint ObserverCheckMCPurity::s_outerHitOfTwo;

/** stores the inner hit of a two-hit-combination. */
SpacePoint ObserverCheckMCPurity::s_innerHitOfTwo;

/** stores the outer hit of a three-hit-combination. */
SpacePoint ObserverCheckMCPurity::s_outerHitOfThree;

/** stores the center hit of a three-hit-combination. */
SpacePoint ObserverCheckMCPurity::s_centerHitOfThree;

/** stores the inner hit of a three-hit-combination. */
SpacePoint ObserverCheckMCPurity::s_innerHitOfThree;

/** dominating mcParticleID. */
int ObserverCheckMCPurity::s_mainMCParticleID = 0;

/** purity for the dominating particleID. */
double ObserverCheckMCPurity::s_mainPurity = -1.;
