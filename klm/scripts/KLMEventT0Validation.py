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
==========================
A basf2.Module that runs after KLMEventT0Estimator in the same path and fills
validation histograms from the estimator's results. It does not recompute them:
it reads what the estimator stores in each event.

  per muon of the ParticleList (extra info; <C> = BKLMScint, BKLMRPC, EKLMScint)
    klmTrackT0<C>, klmTrackT0Err<C>, klmTrackNHits<C>,
    klmTrackSector<C>, klmTrackSection<C>, klmTrackT0All, klmTrackT0ErrAll
  per event
    klmEventT0<C>, klmEventT0Err<C> (EventExtraInfo)
    the KLM entry of EventT0 (final KLM event T0 and its uncertainty)

Only the charge histograms read the hits of the track: the charge of each
digit, before the estimator's charge selection.

Histograms (directory structure):
  diagnostics/
    h_nhits_pertrk_{bklm_scint,bklm_rpc,eklm_scint}
    h_err_pertrk_{bklm_scint,bklm_rpc,eklm_scint}       per-track uncertainty
    h_digitQ_{bklm_scint,eklm_scint}
    h_dimuon_{all,bklm_scint,bklm_rpc,eklm_scint}        T0(mu+) - T0(mu-)
    h_per_track_resolution                              core width of h_dimuon_* / sqrt(2)
  final/
    h_t0evt_klm_final, h_t0evt_klm_final_err
  pulls/
    h_pull_{bklm_scint,bklm_rpc,eklm_scint}             (T0(mu+) - T0(mu-)) / sqrt(err+^2 + err-^2)
    h_pull_{B_vs_E,B_vs_R,E_vs_R}                       two categories of the same track
    h_evt_pull_{B_vs_E,B_vs_R,E_vs_R}                   per-event results of two categories
    sector/{bklm_scint,bklm_rpc,eklm}/                  same-category pulls by sector pair
    sector/h2_pull_{bklm_scint,bklm_rpc,eklm}_{mean,sigma}
  residuals/
    the same as pulls/, in ns

Sector pairs: (i, j) has the mu+ in sector i and the mu- in sector j. BKLM sectors 1-8
(both halves together); EKLM sectors B1-B4 (backward) and F1-F4 (forward).

Usage
-----
  import basf2
  import modularAnalysis as ma
  from KLMEventT0Validation import KLMEventT0ValidationModule

  ma.fillParticleList('mu+:forT0', 'p > 1.5 and abs(d0) < 2 and abs(z0) < 4', path=path)
  path.add_module('KLMEventT0Estimator', MuonListName='mu+:forT0')
  path.add_module(KLMEventT0ValidationModule(muon_list_name='mu+:forT0',
                                             output_file='KLMEventT0Validation.root'))
"""

import array
import math

import basf2
import ROOT
from ROOT import Belle2, TFile, TH1D, TH2D

ROOT.gROOT.SetBatch(True)

#: Estimator categories: (extra info name, histogram suffix, short label)
CATEGORIES = [('BKLMScint', 'bklm_scint', 'B'),
              ('BKLMRPC', 'bklm_rpc', 'R'),
              ('EKLMScint', 'eklm_scint', 'E')]
#: Category pairs compared with each other
CROSS_PAIRS = [('B', 'E'), ('B', 'R'), ('E', 'R')]
#: Number of sector bins per category in the sector-pair maps
N_SECTOR_BINS = {'B': 8, 'R': 8, 'E': 8}


def _sector_index(label, track):
    """Sector-pair index of a track: BKLM sector - 1; EKLM 4 * (section - 1) + (sector - 1)."""
    if label == 'E':
        return 4 * (track['section'] - 1) + (track['sector'] - 1)
    return track['sector'] - 1


def _sector_name(label, index):
    """Readable sector name for histogram names and titles."""
    if label == 'E':
        return ('B' if index < 4 else 'F') + str(index % 4 + 1)
    return str(index + 1)


def core_fit(hist):
    """Single Gaussian fit to the core: start at the median with width IQR/1.349 and fit
    within +-2.5 of that width. Returns (mean, mean error, sigma, sigma error) or None."""
    if hist is None or hist.GetEntries() < 50:
        return None
    probs = array.array('d', [0.25, 0.5, 0.75])
    q = array.array('d', [0.0, 0.0, 0.0])
    hist.GetQuantiles(3, q, probs)
    sigma0 = (q[2] - q[0]) / 1.349
    if not sigma0 > 0.0:
        return None
    f = ROOT.TF1('core_fit', 'gaus', q[1] - 2.5 * sigma0, q[1] + 2.5 * sigma0)
    f.SetParameters(hist.GetMaximum(), q[1], sigma0)
    if int(hist.Fit(f, 'RQN0')) != 0:
        return None
    return f.GetParameter(1), f.GetParError(1), abs(f.GetParameter(2)), f.GetParError(2)


class KLMEventT0ValidationModule(basf2.Module):
    """Validation histograms for KLMEventT0Estimator, filled from the estimator's stored results."""

    def __init__(self, muon_list_name='mu+:forT0', output_file='KLMEventT0Validation.root',
                 opposite_charges_only=True):
        """
        Parameters
        ----------
        muon_list_name : str
            ParticleList given to KLMEventT0Estimator as MuonListName.
        output_file : str
            ROOT file for the histograms.
        opposite_charges_only : bool
            Use only opposite-charge track pairs for the same-category pulls, residuals and sector maps.
        """
        super().__init__()
        #: ParticleList name
        self._muon_list_name = muon_list_name
        #: Output file name
        self._output_file_path = output_file
        #: Opposite-charge pairs only
        self._opp_charges_only = opposite_charges_only
        #: Histograms by key
        self._h = {}
        #: Output file
        self._outfile = None
        #: KLM channel status, for the charge histograms
        self._channel_status = None

    # ------------------------------------------------------------------
    def initialize(self):
        """Open the output file and book the histograms."""
        self._channel_status = Belle2.PyDBObj('KLMChannelStatus')
        self._outfile = TFile(self._output_file_path, 'RECREATE')
        self._book_histograms()

    def terminate(self):
        """Fill the summary histograms, write and close the file."""
        self._fill_summaries()
        self._outfile.Write('', ROOT.TObject.kOverwrite)
        self._outfile.Close()
        basf2.B2INFO(f'KLMEventT0ValidationModule: wrote {self._output_file_path}')

    # ------------------------------------------------------------------
    def _cd(self, path):
        """Create (if needed) and change to a directory of the output file."""
        d = self._outfile
        for part in path.split('/'):
            sub = d.GetDirectory(part)
            d = sub if sub else d.mkdir(part)
        d.cd()

    def _book_histograms(self):
        """Book all histograms; the binning follows the histograms used for the KLM event T0 note."""
        h = self._h

        self._cd('diagnostics')
        for name, sfx, lab in CATEGORIES:
            h[f'nhits_{lab}'] = TH1D(f'h_nhits_pertrk_{sfx}', f'Hits per track ({sfx});hits', 100, -0.5, 99.5)
            h[f'err_{lab}'] = TH1D(f'h_err_pertrk_{sfx}', f'Per-track T0 uncertainty ({sfx});uncertainty [ns]',
                                   800, 0.0, 20.0)
            h[f'dimuon_{lab}'] = TH1D(f'h_dimuon_{sfx}', f'Di-muon T0(#mu^{{+}}) - T0(#mu^{{-}}) ({sfx});#DeltaT_{{0}} [ns]',
                                      800, -100, 100)
        h['dimuon_all'] = TH1D('h_dimuon_all', 'Di-muon T0(#mu^{+}) - T0(#mu^{-}) (all categories);#DeltaT_{0} [ns]',
                               800, -100, 100)
        h['digitQ_B'] = TH1D('h_digitQ_bklm_scint', 'Digit charge (BKLM scintillator);charge [ADC counts]', 100, 0.0, 800.0)
        h['digitQ_E'] = TH1D('h_digitQ_eklm_scint', 'Digit charge (EKLM scintillator);charge [ADC counts]', 100, 0.0, 800.0)
        h['resolution'] = TH1D('h_per_track_resolution', 'Per-track resolution from di-muon #DeltaT_{0};;#sigma/#sqrt{2} [ns]',
                               4, 0.5, 4.5)
        for i, lab in enumerate(['all', 'bklm_scint', 'bklm_rpc', 'eklm_scint'], 1):
            h['resolution'].GetXaxis().SetBinLabel(i, lab)

        self._cd('final')
        h['final'] = TH1D('h_t0evt_klm_final', 'Final KLM event T0;T0 [ns]', 800, -100, 100)
        h['final_err'] = TH1D('h_t0evt_klm_final_err', 'Final KLM event T0 uncertainty;uncertainty [ns]', 800, 0.0, 20.0)

        for kind, title, nb, lo, hi in (('pull', 'Pull', 200, -10, 10), ('res', 'Residual', 500, -25, 25)):
            top = 'pulls' if kind == 'pull' else 'residuals'
            unit = '' if kind == 'pull' else ' [ns]'
            self._cd(top)
            for name, sfx, lab in CATEGORIES:
                h[f'{kind}_{lab}'] = TH1D(f'h_{kind}_{sfx}', f'{title} ({sfx});T0(#mu^{{+}}) - T0(#mu^{{-}}){unit}',
                                          nb, lo, hi)
            for a, b in CROSS_PAIRS:
                h[f'{kind}_{a}_vs_{b}'] = TH1D(f'h_{kind}_{a}_vs_{b}', f'{title}, same track;T0({a}) - T0({b}){unit}',
                                               nb, lo, hi)
                h[f'evt_{kind}_{a}_vs_{b}'] = TH1D(f'h_evt_{kind}_{a}_vs_{b}', f'{title}, per event;T0({a}) - T0({b}){unit}',
                                                   nb, lo, hi)
            # Sector pairs, same binning as the global histograms of the old sector analysis
            snb, slo, shi = (200, -10, 10) if kind == 'pull' else (200, -50, 50)
            for name, sfx, lab in CATEGORIES:
                self._cd(f'{top}/sector/{sfx}')
                n = N_SECTOR_BINS[lab]
                grid = {}
                for i in range(n):
                    for j in range(n):
                        si, sj = _sector_name(lab, i), _sector_name(lab, j)
                        grid[(i, j)] = TH1D(f'h_{kind}_{sfx}_{si}_vs_{sj}',
                                            f'{title} ({sfx}), #mu^{{+}} in {si}, #mu^{{-}} in {sj};'
                                            f'T0(#mu^{{+}}) - T0(#mu^{{-}}){unit}', snb, slo, shi)
                h[f'{kind}_sector_{lab}'] = grid
                self._cd(f'{top}/sector')
                for stat in ('mean', 'sigma'):
                    h2 = TH2D(f'h2_{kind}_{sfx}_{stat}', f'{title} {stat} ({sfx});#mu^{{+}} sector;#mu^{{-}} sector',
                              n, 0.5, n + 0.5, n, 0.5, n + 0.5)
                    for k in range(n):
                        h2.GetXaxis().SetBinLabel(k + 1, _sector_name(lab, k))
                        h2.GetYaxis().SetBinLabel(k + 1, _sector_name(lab, k))
                    h[f'{kind}_sector_{lab}_{stat}'] = h2

    # ------------------------------------------------------------------
    def event(self):
        """Read the estimator's results for this event and fill the histograms."""
        h = self._h
        plist = Belle2.PyStoreObj(self._muon_list_name)
        if not plist.isValid():
            return

        tracks = {lab: [] for _, _, lab in CATEGORIES}   # per category: one dict per track
        all_tracks = []                                   # (charge, T0 over all categories)
        for i in range(plist.obj().getListSize()):
            particle = plist.obj().getParticle(i)
            charge = particle.getCharge()
            for name, sfx, lab in CATEGORIES:
                if not particle.hasExtraInfo('klmTrackT0' + name):
                    continue
                t = {'mu': particle.getExtraInfo('klmTrackT0' + name),
                     'err': particle.getExtraInfo('klmTrackT0Err' + name),
                     'n': int(particle.getExtraInfo('klmTrackNHits' + name)),
                     'sector': int(particle.getExtraInfo('klmTrackSector' + name)),
                     'section': int(particle.getExtraInfo('klmTrackSection' + name)),
                     'charge': charge, 'index': i}
                tracks[lab].append(t)
                h[f'nhits_{lab}'].Fill(t['n'])
                h[f'err_{lab}'].Fill(t['err'])
            if particle.hasExtraInfo('klmTrackT0All'):
                all_tracks.append((charge, particle.getExtraInfo('klmTrackT0All')))
            self._fill_charges(particle)

        # Final KLM event T0
        evt_t0 = Belle2.PyStoreObj('EventT0')
        if evt_t0.isValid():
            klm = evt_t0.obj().getTemporaryEventT0s(Belle2.Const.KLM)
            if klm.size() > 0:
                h['final'].Fill(klm.back().eventT0)
                h['final_err'].Fill(klm.back().eventT0Uncertainty)

        # Same-category pairs: pulls, residuals and sector maps
        for name, sfx, lab in CATEGORIES:
            self._fill_pairs(lab, tracks[lab])

        # Two categories of the same track
        for a, b in CROSS_PAIRS:
            for ta in tracks[a]:
                for tb in tracks[b]:
                    if tb['index'] == ta['index']:
                        self._fill_diff(ta['mu'], ta['err'], tb['mu'], tb['err'], f'_{a}_vs_{b}')

        # Per-event results of two categories
        extra = Belle2.PyStoreObj('EventExtraInfo')
        if extra.isValid():
            evt = {}
            for name, sfx, lab in CATEGORIES:
                if extra.obj().hasExtraInfo('klmEventT0' + name):
                    evt[lab] = (extra.obj().getExtraInfo('klmEventT0' + name),
                                extra.obj().getExtraInfo('klmEventT0Err' + name))
            for a, b in CROSS_PAIRS:
                if a in evt and b in evt:
                    self._fill_diff(evt[a][0], evt[a][1], evt[b][0], evt[b][1], f'_{a}_vs_{b}', prefix='evt_')

        # Di-muon T0(mu+) - T0(mu-): the last track of each charge, as in the estimator's earlier version
        def _last(pairs, sign):
            vals = [mu for q, mu in pairs if q * sign > 0]
            return vals[-1] if vals else None
        pos, neg = _last(all_tracks, 1), _last(all_tracks, -1)
        if pos is not None and neg is not None:
            h['dimuon_all'].Fill(pos - neg)
        for name, sfx, lab in CATEGORIES:
            pairs = [(t['charge'], t['mu']) for t in tracks[lab]]
            pos, neg = _last(pairs, 1), _last(pairs, -1)
            if pos is not None and neg is not None:
                h[f'dimuon_{lab}'].Fill(pos - neg)

    # ------------------------------------------------------------------
    def _fill_diff(self, mu_a, err_a, mu_b, err_b, suffix, prefix=''):
        """Fill residual and pull of a - b."""
        h = self._h
        if not (math.isfinite(mu_a) and math.isfinite(mu_b)):
            return
        h[f'{prefix}res{suffix}'].Fill(mu_a - mu_b)
        den2 = err_a * err_a + err_b * err_b
        if den2 > 0.0:
            h[f'{prefix}pull{suffix}'].Fill((mu_a - mu_b) / math.sqrt(den2))

    def _fill_pairs(self, lab, trk):
        """Same-category pairs of tracks, the mu+ first."""
        h = self._h
        for i in range(len(trk)):
            for j in range(i + 1, len(trk)):
                a, b = trk[i], trk[j]
                if self._opp_charges_only and a['charge'] * b['charge'] >= 0:
                    continue
                if a['charge'] < 0 < b['charge']:
                    a, b = b, a
                den2 = a['err'] ** 2 + b['err'] ** 2
                if not den2 > 0.0:
                    continue
                d = a['mu'] - b['mu']
                pull = d / math.sqrt(den2)
                h[f'res_{lab}'].Fill(d)
                h[f'pull_{lab}'].Fill(pull)
                ki, kj = _sector_index(lab, a), _sector_index(lab, b)
                n = N_SECTOR_BINS[lab]
                if 0 <= ki < n and 0 <= kj < n:
                    h[f'res_sector_{lab}'][(ki, kj)].Fill(d)
                    h[f'pull_sector_{lab}'][(ki, kj)].Fill(pull)

    def _fill_charges(self, particle):
        """Charge of the track's scintillator digits before the charge selection (good digits, normal channels)."""
        track = particle.getTrack()
        if not track:
            return
        status = self._channel_status

        def normal(digit):
            return (not status.isValid() or
                    status.obj().getChannelStatus(digit.getUniqueChannelID()) == Belle2.KLMChannelStatus.c_Normal)

        for hit2d in track.getRelationsTo['Belle2::KLMHit2d']('KLMHit2ds'):
            if hit2d.getSubdetector() == Belle2.KLMElementNumbers.c_EKLM:
                for digit in hit2d.getRelationsTo['Belle2::KLMDigit']('KLMDigits'):
                    if digit.isGood() and normal(digit):
                        self._h['digitQ_E'].Fill(digit.getCharge())
            elif not hit2d.inRPC() and not hit2d.isOutOfTime():
                for hit1d in hit2d.getRelationsTo['Belle2::BKLMHit1d']('BKLMHit1ds'):
                    for digit in hit1d.getRelationsTo['Belle2::KLMDigit']('KLMDigits'):
                        if not digit.inRPC() and digit.isGood() and normal(digit):
                            self._h['digitQ_B'].Fill(digit.getCharge())

    def _fill_summaries(self):
        """Core-fit mean and width of every sector pair, and the per-track resolution."""
        h = self._h
        for kind in ('pull', 'res'):
            for name, sfx, lab in CATEGORIES:
                for (i, j), hh in h[f'{kind}_sector_{lab}'].items():
                    fit = core_fit(hh)
                    if fit is None:
                        continue
                    h[f'{kind}_sector_{lab}_mean'].SetBinContent(i + 1, j + 1, fit[0])
                    h[f'{kind}_sector_{lab}_mean'].SetBinError(i + 1, j + 1, fit[1])
                    h[f'{kind}_sector_{lab}_sigma'].SetBinContent(i + 1, j + 1, fit[2])
                    h[f'{kind}_sector_{lab}_sigma'].SetBinError(i + 1, j + 1, fit[3])
        for b, key in enumerate(['dimuon_all', 'dimuon_B', 'dimuon_R', 'dimuon_E'], 1):
            fit = core_fit(h[key])
            if fit is not None:
                h['resolution'].SetBinContent(b, fit[2] / math.sqrt(2.0))
                h['resolution'].SetBinError(b, fit[3] / math.sqrt(2.0))
