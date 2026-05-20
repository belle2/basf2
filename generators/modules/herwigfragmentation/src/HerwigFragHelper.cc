/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/modules/herwigfragmentation/HerwigFragHelper.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <HepMC/GenEvent.h>

#include <Math/Vector4D.h>
#include <unordered_map>

using namespace Belle2;

HerwigFragHelper::HerwigFragHelper() = default;
HerwigFragHelper::~HerwigFragHelper() = default;

// Since parents can appear after children in HepMC ordering, we apply a two-pass merge:
// pass 1 builds the barcode -> graph-index map;
// pass 2 appends the particles and resolves parent links.
int HerwigFragHelper::addHepMCToGraph(HepMC::GenEvent* evt,
                                      MCParticleGraph& graph,
                                      const std::vector<int>& quarkIndices)
{
  if (!evt) { B2ERROR("HerwigFragHelper: null GenEvent passed"); return -1; }

  const int nparticles = evt->particles_size();
  if (nparticles <= 0) { B2ERROR("HepMC event has no particles"); return -1; }

  const int event_offset = graph.size();
  int n_added = 0;
  std::unordered_map<int, int> hash_index_map;

  // Build a map that takes barcode (HepMC format) -> graph-index (MCParticleGraph format).
  // Done in two passes: identical conditions in both passes guarantees correct parent indices.

  // First pass: build barcode map
  {
    int mapped_count = 0;
    auto it = evt->particles_begin();
    for (int i = 0; i < nparticles; ++i, ++it) {
      const int st  = (*it)->status();
      const int pdg = (*it)->pdg_id();
      if (st == 4 || st < 1) continue;
      // PDG codes 81-99 are non-physical Herwig internal codes (undecayed clusters).
      // Skip in both passes so the barcode->index map stays consistent.
      if (pdg >= 81 && pdg <= 99) continue;
      hash_index_map[(*it)->barcode()] = event_offset + mapped_count;
      mapped_count++;
    }
  }

  const double len_conv = HepMC::Units::conversion_factor(evt->length_unit(), HepMC::Units::CM);
  const double mom_conv = HepMC::Units::conversion_factor(evt->momentum_unit(), HepMC::Units::GEV);

  // Second pass: add particles to graph
  auto rp = evt->particles_begin();
  for (int i = 0; i < nparticles; ++i, ++rp) {
    auto* dv = (*rp)->end_vertex();
    auto* pv = (*rp)->production_vertex();
    const int status   = (*rp)->status();
    const int pdg_code = (*rp)->pdg_id();

    if (status == 4 || status < 1) continue;  // identical to first pass
    if (pdg_code >= 81 && pdg_code <= 99) continue;  // identical to first pass

    graph.addParticle();
    MCParticleGraph::GraphParticle& p = graph[event_offset + n_added];

    p.setPDG(pdg_code);

    auto mom = (*rp)->momentum();
    p.setMomentum(ROOT::Math::XYZVector(
                    mom.x() * mom_conv * Unit::GeV,
                    mom.y() * mom_conv * Unit::GeV,
                    mom.z() * mom_conv * Unit::GeV));
    p.setEnergy(mom.t() * mom_conv * Unit::GeV);
    p.setMass((*rp)->generated_mass() * mom_conv);

    if (pv) {
      auto pos = pv->position();
      p.setProductionVertex(ROOT::Math::XYZVector(pos.x(), pos.y(), pos.z()) * len_conv * Unit::cm);
      p.setProductionTime(pos.t() * len_conv * Unit::cm / Const::speedOfLight);
      p.setValidVertex(true);
    }

    p.addStatus(MCParticle::c_PrimaryParticle);

    const bool isFinal = !dv && status == 1;
    if (isFinal)                     p.addStatus(MCParticleGraph::GraphParticle::c_StableInGenerator);
    if (pdg_code == 22 && isFinal)   p.addStatus(MCParticleGraph::GraphParticle::c_IsFSRPhoton);

    // setVirtual() status code set - full set of virtual status codes
    const bool isVirtual = (status == 2 || status == 21 || status == 22 ||
                            status == 23 || status == 51 || status == 52);
    if (isVirtual) p.setVirtual();

    // Parent linking: barcode map first, fall back to PDG-code match for beam/skipped parents
    if (pv) {
      auto par_it = pv->particles_begin(HepMC::parents);
      if (par_it != pv->particles_end(HepMC::parents)) {
        auto found = hash_index_map.find((*par_it)->barcode());
        if (found != hash_index_map.end()) {
          const int pidx = found->second;
          const int sidx = event_offset + n_added;
          if (static_cast<size_t>(pidx) < graph.size() && pidx != sidx)
            p.comesFrom(graph[pidx]);
        } else {
          bool linked = false;
          for (int qidx : quarkIndices)
            if (static_cast<size_t>(qidx) < graph.size() && graph[qidx].getPDG() == pdg_code)
            { p.comesFrom(graph[qidx]); linked = true; break; }
          if (!linked && !quarkIndices.empty())
            p.comesFrom(graph[quarkIndices[0]]);
        }
      }
    }

    n_added++;
  }

  // 4-momentum conservation debug check
  {
    double fsE = 0, fsPx = 0, fsPy = 0, fsPz = 0;
    for (auto pit = evt->particles_begin(); pit != evt->particles_end(); ++pit) {
      if ((*pit)->status() != 1) continue;
      const auto& m2 = (*pit)->momentum();
      fsE += m2.t() * mom_conv; fsPx += m2.x() * mom_conv;
      fsPy += m2.y() * mom_conv; fsPz += m2.z() * mom_conv;
    }
    const double fsM2 = fsE * fsE - fsPx * fsPx - fsPy * fsPy - fsPz * fsPz;
    B2DEBUG(10, "Post-Herwig final-state sum: E=" << fsE
            << " M_inv=" << (fsM2 > 0.0 ? std::sqrt(fsM2) : 0.0));
  }

  B2DEBUG(10, "Added " << n_added << " particles to graph");
  return n_added;
}
