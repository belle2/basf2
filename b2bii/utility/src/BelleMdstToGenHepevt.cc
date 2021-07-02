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
// Original file: ${BELLE_TOP_DIR}/src/anal/mdst/src/mdst.cc
//
// Ported by: A. Zupanc (March 2015)
//
// ****************************************************************

#include <b2bii/utility/BelleMdstToGenHepevt.h>

// trace back to isthep>=0 or isthep=-10(decay-in-flight)
static const Belle::Gen_hepevt& gen_level_step1(const Belle::Gen_hepevt& gen)
{
  return (gen && gen.mother() && gen.isthep() < 0 && gen.isthep() != -10)
         ? gen_level_step1(gen.mother()) : gen;
}

// trace back to isthep>0 level
static const Belle::Gen_hepevt& gen_level_step2(const Belle::Gen_hepevt& gen)
{
  return (gen && gen.mother() && gen.isthep() < 0)
         ? gen_level_step2(gen.mother()) : gen;
}

const Belle::Gen_hepevt& gen_level(const Belle::Gen_hepevt& gen)
{
  const Belle::Gen_hepevt& level1(gen_level_step1(gen));
  if (!level1 || level1.isthep() >= 0) {
    return level1;
  } else if (level1.isthep() == -10) {
    const Belle::Gen_hepevt& level2(gen_level_step2(gen));
    // If "gen" is made by Ks(which is made by generator) daughter, returns daughter
    if (level2.idhep() == 310 || abs(level2.idhep()) == 3122) return level1;
    // Otherwize, trace back to generator level
    else return level2;
  }
  return level1;
}

const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_trk& trk, int i)
{
  Belle::Mdst_sim_trk_Manager& xrefmgr = Belle::Mdst_sim_trk_Manager::get_manager();
  Belle::Mdst_sim_trk_Index index(xrefmgr.index("trk"));
  index.update();
  Belle::Panther_ID trackID(trk.get_ID());
  std::vector<Belle::Mdst_sim_trk> point = point_from(trackID, index);
  if (i >= 0 && i < (int)point.size())  return point[i].hepevt();
  else return Belle::Gen_hepevt_Manager::get_manager().get_NULL();
}

const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_charged& ch, int i)
{
  if (!Belle::Mdst_sim_trk_Manager::get_manager().count()) {
    // for backward compatibility
    Belle::Mdst_sim_xref_Manager& xrefmgr = Belle::Mdst_sim_xref_Manager::get_manager();
    Belle::Mdst_sim_xref_Index index(xrefmgr.index("charged"));
    index.update();
    Belle::Panther_ID trackID(ch.get_ID());
    std::vector<Belle::Mdst_sim_xref> point = point_from(trackID, index);
    if (i >= 0 && i < (int)point.size())  return point[i].hepevt();
    else return Belle::Gen_hepevt_Manager::get_manager().get_NULL();
  }
  return get_hepevt(ch.trk(), i);
}

const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_ecl& ecl, int i)
{
  Belle::Mdst_sim_ecl_Manager& xrefmgr = Belle::Mdst_sim_ecl_Manager::get_manager();
  Belle::Mdst_sim_ecl_Index index(xrefmgr.index("ecl"));
  index.update();
  Belle::Panther_ID eclID(ecl.get_ID());
  std::vector<Belle::Mdst_sim_ecl> point = point_from(eclID, index);
  if (i >= 0 && i < (int)point.size())  return point[i].hepevt();
  else return Belle::Gen_hepevt_Manager::get_manager().get_NULL();
}

const Belle::Gen_hepevt& get_hepevt(const Belle::Mdst_gamma& gamma, int i)
{
  if (!Belle::Mdst_sim_ecl_Manager::get_manager().count()) {
    // do NOT support backward compatibility
    // because the returned "Belle::Gen_hepevt & " may not be correct
    return Belle::Gen_hepevt_Manager::get_manager().get_NULL();
  }
  return get_hepevt(gamma.ecl(), i);
}

