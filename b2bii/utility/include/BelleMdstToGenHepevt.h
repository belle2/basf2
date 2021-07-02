/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// ****************************************************************
// Belle MDST objects -> MC Particle relations
//
// Original file: ${BELLE_TOP_DIR}/src/anal/mdst/mdst/mdst.h
//
// Ported by: A. Zupanc (March 2015)
//
// ****************************************************************

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"

#include "belle_legacy/tables/hepevt.h"
#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/mdst_obs.h"

/*
class Belle::Mdst_charged;
class Belle::Mdst_ecl;
class Belle::Mdst_gamma;
class Belle::Gen_hepevt;
class Belle::Mdst_trk;*/

/**
 * Returns reference to MC matched Belle::Gen_hepevt object
 */
const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_trk&, int ith = 0);

/**
 * Returns reference to MC matched Belle::Gen_hepevt object
 */
const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_charged&, int ith = 0);

/**
 * Returns reference to MC matched Belle::Gen_hepevt object
 *
 * returns correponding hepevt(This hepevt might be made at inside of ECL).
 * the following example will return what most of you want!
 * Belle::Mdst_ecl & e = ...
 * const Belle::Gen_hepevt & h(Belle::Gen_level(get_hepevt(e)));
 *
 */
const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_ecl&, int ith = 0);

/**
 * Returns reference to MC matched Belle::Gen_hepevt object
 *
 * returns correponding hepevt(This hepevt might be made at inside of ECL).
 * the following example will return what most of you want!
 * Belle::Mdst_ecl & e = ...
 * const Belle::Gen_hepevt & h(Belle::Gen_level(get_hepevt(e)));
 *
 */
const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_gamma&, int ith = 0);

/**
 * trace back to mother hepevt which has isthep>0 or
 * isthep =-10(in case of daughters of Ks/Lambda)
 */
const Belle::Gen_hepevt& gen_level(const Belle::Gen_hepevt&);

