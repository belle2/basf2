#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                 #
##########################################################################

"""
KLMEventT0ValidationModule
===========================
A basf2.Module subclass that runs in the same basf2 path as
KLMEventT0Estimator and fills validation histograms for the per-track
T0 estimation quality.

Distributions produced:

  pulls/
    Pull_BKLM_Scint           - (T0_i - T0_j) / sqrt(SEM_i^2 + SEM_j^2)
    Pull_EKLM_Scint
    Pull_BKLM_RPC

  residuals/
    Residual_BKLM_Scint       - raw ΔT0 = T0_i - T0_j between tracks
    Residual_EKLM_Scint
    Residual_BKLM_RPC

  cross_detector/
    DeltaT0_BKLMScint_vs_EKLMScint
    DeltaT0_BKLMScint_vs_BKLMRPC
    DeltaT0_EKLMScint_vs_BKLMRPC

  dimuon/
    DimuonDeltaT0_All         - T0(mu+) - T0(mu-) using all KLM hits
    DimuonDeltaT0_BKLMScint
    DimuonDeltaT0_BKLMScint
    DimuonDeltaT0_BKLMRPC
    DimuonDeltaT0_EKLMScint

Usage
-----
  from klm.validation.KLMEventT0Validation import KLMEventT0ValidationModule
  path.add_module(KLMEventT0ValidationModule(
      muon_list_name='mu+:forT0',
      output_file='KLMEventT0Validation.root',
  ))

Implementation note
-------------------
Per-track T0 is estimated as::

    T0_est(hit) = hit.getTime() - ExtHit.getFlightTime()

Flight time (from the extrapolated-hit objects attached to each track)
is the dominant correction. Cable-delay and propagation-time corrections
require conditions-DB access and are omitted here; they produce a common
per-track offset that largely cancels in the DIFFERENCE T0_i - T0_j used
for all validation distributions.
"""

import math

import basf2
import ROOT
from ROOT import Belle2, TFile, TH1D, gROOT

gROOT.SetBatch(True)


# ---------------------------------------------------------------------------
# Small statistics helpers
# ---------------------------------------------------------------------------

def _mu_sem(values):
    """Return (mean, SEM) for a list of floats, or (None, None) if empty."""
    n = len(values)
    if n == 0:
        return None, None
    mu = sum(values) / n
    if n == 1:
        return mu, 0.0
    ss = sum((x - mu) ** 2 for x in values)
    return mu, math.sqrt(ss / (n - 1) / n)


def _weighted_mean_sem(pairs):
    """
    Inverse-variance weighted mean and combined uncertainty.

    Parameters
    ----------
    pairs : list of (mu, sem) tuples

    Returns
    -------
    (mu, sem) or (None, None)
    """
    if not pairs:
        return None, None
    if len(pairs) == 1:
        mu, se = pairs[0]
        return mu, (se if se is not None else 0.0)
    valid = [(mu, se) for mu, se in pairs if se is not None and se > 0.0]
    if valid:
        wsum = sum(1.0 / (se * se) for _, se in valid)
        wtsum = sum(mu / (se * se) for mu, se in valid)
        if wsum > 0.0:
            return wtsum / wsum, math.sqrt(1.0 / wsum)
    # Fallback: simple mean
    vals = [mu for mu, _ in pairs]
    mu = sum(vals) / len(vals)
    return mu, None


# ---------------------------------------------------------------------------
# Module class
# ---------------------------------------------------------------------------

class KLMEventT0ValidationModule(basf2.Module):
    """
    Validation module for KLMEventT0Estimator.

    Reads muon tracks from a ParticleList, estimates per-track T0 by
    category (BKLM Scint, BKLM RPC, EKLM Scint), then fills pull,
    residual, cross-detector ΔT0, and dimuon ΔT0 histograms.
    """

    def __init__(self,
                 muon_list_name: str = 'mu+:forT0',
                 output_file: str = 'KLMEventT0Validation.root',
                 opposite_charges_only: bool = True):
        """
        Parameters
        ----------
        muon_list_name : str
            Name of the ParticleList of muon candidates (must match the
            name used in KLMEventT0Estimator's MuonListName parameter).
        output_file : str
            Path of the output ROOT file.
        opposite_charges_only : bool
            If True, only form pairs with opposite-sign charges (dimuon
            selection). If False, all track pairs are used.
        """
        super().__init__()
        self.set_name('KLMEventT0ValidationModule')
        # Name of the muon ParticleList.
        self._muon_list_name = muon_list_name
        # Path of the output ROOT file.
        self._output_file_path = output_file
        # If True, only use opposite-charge track pairs.
        self._opp_charges_only = opposite_charges_only

        # TFile object for the output ROOT file.
        self._output_file = None
        # Dictionary of booked ROOT histograms keyed by short name.
        self._histograms = {}

    # ------------------------------------------------------------------
    def initialize(self):
        """Open output file and book histograms."""
        self._output_file = TFile(self._output_file_path, 'RECREATE')
        self._book_histograms()

    # ------------------------------------------------------------------
    def _book_histograms(self):
        """Create and register all validation histograms in the output file."""
        h = self._histograms
        f = self._output_file

        f.mkdir('pulls').cd()
        h['pull_B'] = TH1D('Pull_BKLM_Scint',
                           'Pull BKLM Scint;(T_{0,i} - T_{0,j}) / #sigma;entries',
                           100, -10.0, 10.0)
        h['pull_E'] = TH1D('Pull_EKLM_Scint',
                           'Pull EKLM Scint;(T_{0,i} - T_{0,j}) / #sigma;entries',
                           100, -10.0, 10.0)
        h['pull_R'] = TH1D('Pull_BKLM_RPC',
                           'Pull BKLM RPC;(T_{0,i} - T_{0,j}) / #sigma;entries',
                           100, -10.0, 10.0)

        f.mkdir('residuals').cd()
        h['res_B'] = TH1D('Residual_BKLM_Scint',
                          'Residual BKLM Scint;#DeltaT_{0} (ns);entries',
                          200, -50.0, 50.0)
        h['res_E'] = TH1D('Residual_EKLM_Scint',
                          'Residual EKLM Scint;#DeltaT_{0} (ns);entries',
                          200, -50.0, 50.0)
        h['res_R'] = TH1D('Residual_BKLM_RPC',
                          'Residual BKLM RPC;#DeltaT_{0} (ns);entries',
                          200, -50.0, 50.0)

        f.mkdir('cross_detector').cd()
        h['res_B_vs_E'] = TH1D('DeltaT0_BKLMScint_vs_EKLMScint',
                               'BKLM Scint - EKLM Scint;#DeltaT_{0} (ns);entries',
                               200, -50.0, 50.0)
        h['res_B_vs_R'] = TH1D('DeltaT0_BKLMScint_vs_BKLMRPC',
                               'BKLM Scint - BKLM RPC;#DeltaT_{0} (ns);entries',
                               200, -50.0, 50.0)
        h['res_E_vs_R'] = TH1D('DeltaT0_EKLMScint_vs_BKLMRPC',
                               'EKLM Scint - BKLM RPC;#DeltaT_{0} (ns);entries',
                               200, -50.0, 50.0)

        f.mkdir('dimuon').cd()
        h['dimuon_all'] = TH1D('DimuonDeltaT0_All',
                               'Dimuon #DeltaT_{0} all KLM;T_{0}(#mu^{+}) - T_{0}(#mu^{-}) (ns);entries',
                               200, -50.0, 50.0)
        h['dimuon_B'] = TH1D('DimuonDeltaT0_BKLMScint',
                             'Dimuon #DeltaT_{0} BKLM Scint;T_{0}(#mu^{+}) - T_{0}(#mu^{-}) (ns);entries',
                             200, -50.0, 50.0)
        h['dimuon_R'] = TH1D('DimuonDeltaT0_BKLMRPC',
                             'Dimuon #DeltaT_{0} BKLM RPC;T_{0}(#mu^{+}) - T_{0}(#mu^{-}) (ns);entries',
                             200, -50.0, 50.0)
        h['dimuon_E'] = TH1D('DimuonDeltaT0_EKLMScint',
                             'Dimuon #DeltaT_{0} EKLM Scint;T_{0}(#mu^{+}) - T_{0}(#mu^{-}) (ns);entries',
                             200, -50.0, 50.0)

        # Prevent ROOT garbage-collection of objects not owned by directories
        for hist in h.values():
            ROOT.SetOwnership(hist, False)

        f.cd()

    # ------------------------------------------------------------------
    def _mean_klm_flight_time(self, track):
        """
        Return the mean flight time (ns) of all KLM ExtHits for this track.

        This is used as a per-track approximation to the hit-level flight
        time.  When the flight-time correction is the same for all hits on
        a track, the cable-delay + propagation residual is still present
        but cancels in the difference T0_i - T0_j between tracks.
        """
        ext_hits = track.getRelationsTo('ExtHit')
        tof_sum = 0.0
        n = 0
        for ext in ext_hits:
            if ext.getDetectorID() != Belle2.Const.KLM:
                continue
            tof = ext.getFlightTime()
            if math.isfinite(tof) and tof >= 0.0:
                tof_sum += tof
                n += 1
        return tof_sum / n if n > 0 else 0.0

    # ------------------------------------------------------------------
    def _accumulate_track(self, track):
        """
        Compute per-track T0 estimates for each KLM detector category.

        Returns
        -------
        dict with keys 'B' (BKLM Scint), 'R' (BKLM RPC), 'E' (EKLM Scint).
        Each value is (mu, sem) where mu and sem may be None if no hits.
        """
        mean_tof = self._mean_klm_flight_time(track)

        hits_B, hits_R, hits_E = [], [], []
        for hit in track.getRelationsTo('KLMHit2d'):
            t_raw = hit.getTime()
            if not math.isfinite(t_raw):
                continue
            t0_est = t_raw - mean_tof
            subdet = hit.getSubdetector()
            if subdet == Belle2.KLMElementNumbers.c_EKLM:
                hits_E.append(t0_est)
            elif subdet == Belle2.KLMElementNumbers.c_BKLM:
                if hit.inRPC():
                    hits_R.append(t0_est)
                else:
                    hits_B.append(t0_est)

        return {
            'B': _mu_sem(hits_B),
            'R': _mu_sem(hits_R),
            'E': _mu_sem(hits_E),
        }

    # ------------------------------------------------------------------
    def event(self):
        """Read muon tracks, compute per-track T0, fill histograms."""
        muon_list = Belle2.PyStoreObj(self._muon_list_name)
        if not muon_list.isValid():
            return
        n = muon_list.obj().getListSize()
        if n == 0:
            return

        # Per-track T0: lists of (mu, sem, charge)
        trk_B, trk_R, trk_E = [], [], []

        for i in range(n):
            particle = muon_list.obj().getParticle(i)
            if not particle:
                continue
            track = particle.getTrack()
            if not track:
                continue

            charge = int(particle.getCharge())
            cats = self._accumulate_track(track)

            muB, semB = cats['B']
            muR, semR = cats['R']
            muE, semE = cats['E']

            if muB is not None and math.isfinite(muB):
                trk_B.append((muB, semB, charge))
            if muR is not None and math.isfinite(muR):
                trk_R.append((muR, semR, charge))
            if muE is not None and math.isfinite(muE):
                trk_E.append((muE, semE, charge))

        # --- Same-detector pair histograms ---
        def fill_pairs(trk_list, h_pull, h_res):
            n_t = len(trk_list)
            for i in range(n_t):
                mi, si, qi = trk_list[i]
                for j in range(i + 1, n_t):
                    mj, sj, qj = trk_list[j]
                    if self._opp_charges_only and qi * qj >= 0:
                        continue
                    dt = mi - mj
                    if not math.isfinite(dt):
                        continue
                    if h_res:
                        h_res.Fill(dt)
                    den2 = (si * si + sj * sj) if (si and sj) else 0.0
                    if h_pull and den2 > 0.0:
                        pull = dt / math.sqrt(den2)
                        if math.isfinite(pull):
                            h_pull.Fill(pull)

        h = self._histograms
        fill_pairs(trk_B, h.get('pull_B'), h.get('res_B'))
        fill_pairs(trk_R, h.get('pull_R'), h.get('res_R'))
        fill_pairs(trk_E, h.get('pull_E'), h.get('res_E'))

        # --- Cross-detector event-level ΔT0 ---
        def event_t0(trk_list):
            """Weighted mean T0 across all tracks in a category."""
            pairs = [(mu, se) for mu, se, _ in trk_list]
            mu, _ = _weighted_mean_sem(pairs)
            return mu

        muB_evt = event_t0(trk_B)
        muE_evt = event_t0(trk_E)
        muR_evt = event_t0(trk_R)

        def fill_dt0(ha, hb, key):
            if ha is not None and hb is not None:
                d = ha - hb
                if math.isfinite(d) and h.get(key):
                    h[key].Fill(d)

        fill_dt0(muB_evt, muE_evt, 'res_B_vs_E')
        fill_dt0(muB_evt, muR_evt, 'res_B_vs_R')
        fill_dt0(muE_evt, muR_evt, 'res_E_vs_R')

        # --- Dimuon ΔT0 ---
        def dimuon_dt0(trk_list, hist):
            """Fill hist with T0(mu+) - T0(mu-) for the given category."""
            if not hist:
                return
            t0p = t0m = None
            for mu, _, q in trk_list:
                if q > 0 and math.isfinite(mu):
                    t0p = mu
                elif q < 0 and math.isfinite(mu):
                    t0m = mu
            if t0p is not None and t0m is not None:
                d = t0p - t0m
                if math.isfinite(d):
                    hist.Fill(d)

        # All KLM: one entry per charge sign (prefer B, then R, then E)
        trk_all_by_charge = {}
        for lst in (trk_B, trk_E, trk_R):
            for mu, se, q in lst:
                if q not in trk_all_by_charge:
                    trk_all_by_charge[q] = (mu, se, q)
        trk_all = list(trk_all_by_charge.values())

        dimuon_dt0(trk_all, h.get('dimuon_all'))
        dimuon_dt0(trk_B,   h.get('dimuon_B'))
        dimuon_dt0(trk_R,   h.get('dimuon_R'))
        dimuon_dt0(trk_E,   h.get('dimuon_E'))

    # ------------------------------------------------------------------
    def terminate(self):
        """Write all histograms and close the output file."""
        if self._output_file:
            self._output_file.Write('', ROOT.TObject.kOverwrite)
            self._output_file.Close()
            basf2.B2INFO(f'KLMEventT0ValidationModule: wrote {self._output_file_path}')
