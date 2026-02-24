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

Per-hit T0 is computed as::

    T0_est = Trec - Tcable - Tprop - Tfly

where:
    Trec   = KLMDigit::getTime()          (raw digit time)
    Tcable = KLMTimeCableDelay payload     (cable delay per channel)
    Tprop  = dist * delay_const            (propagation along strip,
                                            KLMTimeConstants payload)
    Tfly   = 0.5 * (entry_TOF + exit_TOF) (particle flight time from
                                            matched ExtHit pair)

This exactly mirrors the correction chain in KLMEventT0EstimatorModule.

Histograms produced (directory structure):

  per_track/
    h_t0trk_bklm_scint, h_t0trk_bklm_rpc, h_t0trk_eklm_scint

  per_event/
    h_t0evt_{bklm_scint,bklm_rpc,eklm_scint,all}
    h_final_source

  diagnostics/
    h_nhits_pertrk_{bklm_scint,eklm_scint}
    h_sem_pertrk_{bklm_scint,eklm_scint}
    h_digitQ_{bklm_scint,eklm_scint}
    h_sample_type
    h_T{rec,cable,prop,fly}_{bklm_scint,bklm_rpc,eklm_scint}

  pulls/
    h_pull_{bklm_scint,eklm_scint,rpc_phi,rpc_z,bklm_rpc}
    h_pull_{B_vs_E,B_vs_R,E_vs_R}
    h_pull_summary_{mean,width}
    sector/bklm_scint/  -- 8x8 pairwise pull histograms
    sector/eklm_scint_fwd_vs_bwd/ -- 4x4 pairwise
    sector/rpc_phi/     -- 8x8 pairwise
    sector/rpc_z/       -- 8x8 pairwise
    (+ 2D summary TH2D for each sector group)

  residuals/            -- same structure as pulls but unnormalized (ns)

  cross_detector/
    h_deltaT0_<RegA>_vs_<RegB>  -- 4x4 matrix (4 detector regions)
    h2_deltaT0_{mean,sigma,entries}

  dimuon/
    h_dimuon_{all,bklm_scint,bklm_rpc,eklm_scint,scint_only,with_rpc,with_rpc_dir}
    h_per_track_resolution  -- σ(ΔT0)/√2, 6 categories (filled in terminate)

  final/
    h_t0evt_final_{scint_only,with_rpc,with_rpc_dir}

Usage
-----
  from klm.validation.KLMEventT0Validation import KLMEventT0ValidationModule
  path.add_module(KLMEventT0ValidationModule(
      muon_list_name='mu+:forT0',
      output_file='KLMEventT0Validation.root',
  ))
"""

import math

import basf2
import ROOT
from ROOT import Belle2, TFile, TH1D, TH1I, TH2D, gROOT, TF1

gROOT.SetBatch(True)

# ---------------------------------------------------------------------------
# Constants mirroring the C++ module
# ---------------------------------------------------------------------------
_N_BKLM_SECTORS = 8
_N_EKLM_SECTORS = 4   # per endcap (sectors 1-4)
_N_REGIONS = 4        # EKLM Bwd / EKLM Fwd / BKLM RPC / BKLM Scint
_REG_EKLM_BWD = 0
_REG_EKLM_FWD = 1
_REG_BKLM_RPC = 2
_REG_BKLM_SCINT = 3

# EKLM section numbers: 1 = backward (z<0), 2 = forward (z>0)
_EKLM_SECTION_BWD = 1
_EKLM_SECTION_FWD = 2

# basf2 / CLHEP unit conversions
# basf2 positions are in cm; CLHEP uses mm internally.
# Belle2::Unit::mm = 0.1  (1 mm = 0.1 basf2-units = 0.1 cm)
# CLHEP::mm = 1.0
_BASF2_TO_MM = 10.0    # multiply cm → mm
_MM_TO_BASF2 = 0.1     # multiply mm → cm (= Unit::mm / CLHEP::mm)


# ---------------------------------------------------------------------------
# Small statistics helpers
# ---------------------------------------------------------------------------

def _mu_sem(values):
    """Return (mean, SEM) for a list, or (None, None) if empty."""
    n = len(values)
    if n == 0:
        return None, None
    mu = sum(values) / n
    if n == 1:
        return mu, 0.0
    ss = sum((x - mu) ** 2 for x in values)
    return mu, math.sqrt(ss / (n - 1) / n)


def _weighted_mean(pairs):
    """
    Inverse-variance weighted mean.

    Parameters
    ----------
    pairs : list of (mu, sem) tuples

    Returns
    -------
    (mu, sem) or (None, None)
    """
    if not pairs:
        return None, None
    valid = [(mu, se) for mu, se in pairs
             if se is not None and se > 0.0 and math.isfinite(mu)]
    if valid:
        wsum = sum(1.0 / (se * se) for _, se in valid)
        wtsum = sum(mu / (se * se) for mu, se in valid)
        if wsum > 0.0:
            return wtsum / wsum, math.sqrt(1.0 / wsum)
    # Fallback: simple mean
    vals = [mu for mu, _ in pairs if math.isfinite(mu)]
    if not vals:
        return None, None
    return sum(vals) / len(vals), None


def _gauss_fit(hist, fit_lo=-5.0, fit_hi=5.0):
    """
    Fit a Gaussian to *hist* and return (mean, meanErr, sigma, sigmaErr).
    Returns all None on failure or insufficient statistics.
    """
    if hist is None or hist.GetEntries() < 30:
        return None, None, None, None
    gaus = TF1('_gfit', 'gaus', fit_lo, fit_hi)
    gaus.SetParameters(hist.GetMaximum(), hist.GetMean(), hist.GetStdDev())
    gaus.SetParLimits(2, 0.01, abs(fit_hi - fit_lo))
    status = hist.Fit(gaus, 'LQN0', '', fit_lo, fit_hi)
    if status != 0:
        gaus.Delete()
        return None, None, None, None
    result = (gaus.GetParameter(1), gaus.GetParError(1),
              abs(gaus.GetParameter(2)), gaus.GetParError(2))
    gaus.Delete()
    return result


# ---------------------------------------------------------------------------
# Module class
# ---------------------------------------------------------------------------

class KLMEventT0ValidationModule(basf2.Module):
    """
    Full validation module for KLMEventT0Estimator.

    Recomputes per-track T0 using the same correction chain as the C++
    estimator (Trec - Tcable - Tprop - Tfly) and fills the complete set
    of validation histograms that were present in the monolithic Trash
    module: per-track/per-event T0, timing components, pulls, residuals,
    cross-detector ΔT0, dimuon ΔT0, pairwise sector histograms and their
    2D Gaussian-fit summaries.
    """

    def __init__(self,
                 muon_list_name: str = 'mu+:forT0',
                 output_file: str = 'KLMEventT0Validation.root',
                 opposite_charges_only: bool = True,
                 adc_cut_bklm_scint_min: float = 30.0,
                 adc_cut_bklm_scint_max: float = 320.0,
                 adc_cut_eklm_scint_min: float = 40.0,
                 adc_cut_eklm_scint_max: float = 350.0):
        """Initialise the module with histogram booking parameters and ADC cuts."""
        super().__init__()
        self.set_name('KLMEventT0ValidationModule')
        #: Muon ParticleList name.
        self._muon_list_name = muon_list_name
        #: Output ROOT file path.
        self._output_file_path = output_file
        #: Only use opposite-charge track pairs for pull/residual histograms.
        self._opp_charges_only = opposite_charges_only
        #: Lower ADC cut for BKLM scintillator digits.
        self._adc_bklm_min = adc_cut_bklm_scint_min
        #: Upper ADC cut for BKLM scintillator digits.
        self._adc_bklm_max = adc_cut_bklm_scint_max
        #: Lower ADC cut for EKLM scintillator digits.
        self._adc_eklm_min = adc_cut_eklm_scint_min
        #: Upper ADC cut for EKLM scintillator digits.
        self._adc_eklm_max = adc_cut_eklm_scint_max

        #: Output TFile.
        self._outfile = None
        #: All booked histograms, keyed by short name.
        self._h = {}

        #: BKLM geometry parameters (initialised in initialize()).
        self._geo_bklm = None
        #: EKLM geometry data (initialised in initialize()).
        self._geo_eklm = None
        #: EKLM coordinate transform helper (initialised in initialize()).
        self._transform_eklm = None
        #: EKLM element-number helper (initialised in initialize()).
        self._elem_num = None

        #: Propagation-delay constant for EKLM strips (ns/cm), refreshed in beginRun().
        self._delay_eklm = 0.0
        #: Propagation-delay constant for BKLM scintillator strips (ns/cm), refreshed in beginRun().
        self._delay_bklm = 0.0
        #: Propagation-delay constant for BKLM RPC phi strips (ns/cm), refreshed in beginRun().
        self._delay_rpc_phi = 0.0
        #: Propagation-delay constant for BKLM RPC z strips (ns/cm), refreshed in beginRun().
        self._delay_rpc_z = 0.0
        #: Cable-delay PyDBObj used to query per-channel offsets, refreshed in beginRun().
        self._db_cable_delay = None
        #: Channel-status PyDBObj used to skip bad channels, refreshed in beginRun().
        self._db_channel_status = None

    # ------------------------------------------------------------------
    # Lifecycle
    # ------------------------------------------------------------------

    def initialize(self):
        """Open output file, book histograms, and initialise geometry."""
        self._outfile = TFile(self._output_file_path, 'RECREATE')
        self._book_histograms()

        self._geo_bklm = Belle2.bklm.GeometryPar.instance()
        self._geo_eklm = Belle2.EKLM.GeometryData.Instance()
        self._transform_eklm = Belle2.EKLM.TransformData(
            True, Belle2.EKLM.TransformData.c_None)
        self._elem_num = Belle2.KLMElementNumbers.Instance()

        # Register DB dependencies
        self._db_cable_delay = Belle2.PyDBObj(
            'KLMTimeCableDelay', Belle2.KLMTimeCableDelay)
        self._db_channel_status = Belle2.PyDBObj(
            'KLMChannelStatus', Belle2.KLMChannelStatus)

    def beginRun(self):
        """Cache run-dependent DB constants from KLMTimeConstants payload."""
        tc = Belle2.PyDBObj('KLMTimeConstants', Belle2.KLMTimeConstants)
        if tc.isValid():
            obj = tc.obj()
            self._delay_eklm = obj.getDelay(Belle2.KLMTimeConstants.c_EKLM)
            self._delay_bklm = obj.getDelay(Belle2.KLMTimeConstants.c_BKLM)
            self._delay_rpc_phi = obj.getDelay(Belle2.KLMTimeConstants.c_RPCPhi)
            self._delay_rpc_z = obj.getDelay(Belle2.KLMTimeConstants.c_RPCZ)
        else:
            basf2.B2WARNING('KLMEventT0Validation: KLMTimeConstants not available; '
                            'propagation delays set to zero.')
            self._delay_eklm = self._delay_bklm = 0.0
            self._delay_rpc_phi = self._delay_rpc_z = 0.0

    def terminate(self):
        """Fit summary histograms, write all histograms, and close the file."""
        self._fit_summary_histograms()
        if self._outfile:
            self._outfile.Write('', ROOT.TObject.kOverwrite)
            self._outfile.Close()
            basf2.B2INFO(f'KLMEventT0ValidationModule: wrote {self._output_file_path}')

    # ------------------------------------------------------------------
    # Histogram booking
    # ------------------------------------------------------------------

    def _book_histograms(self):
        """Create all validation histograms inside the output ROOT file."""
        h = self._h
        f = self._outfile

        def H1(name, title, nb, lo, hi):
            return TH1D(name, title, nb, lo, hi)

        def H1I(name, title, nb, lo, hi):
            return TH1I(name, title, nb, lo, hi)

        # --- per_track/ ---
        f.mkdir('per_track').cd()
        h['t0trk_B'] = H1('h_t0trk_bklm_scint',
                          'Per-track T_{0} (BKLM Scint);T_{0} [ns]', 800, -100, 100)
        h['t0trk_R'] = H1('h_t0trk_bklm_rpc',
                          'Per-track T_{0} (BKLM RPC);T_{0} [ns]', 800, -100, 100)
        h['t0trk_E'] = H1('h_t0trk_eklm_scint',
                          'Per-track T_{0} (EKLM Scint);T_{0} [ns]', 800, -100, 100)

        # --- per_event/ ---
        f.mkdir('per_event').cd()
        h['t0evt_B'] = H1('h_t0evt_bklm_scint',
                          'Per-event T_{0} (BKLM Scint);T_{0} [ns]', 800, -100, 100)
        h['t0evt_R'] = H1('h_t0evt_bklm_rpc',
                          'Per-event T_{0} (BKLM RPC);T_{0} [ns]', 800, -100, 100)
        h['t0evt_E'] = H1('h_t0evt_eklm_scint',
                          'Per-event T_{0} (EKLM Scint);T_{0} [ns]', 800, -100, 100)
        h['t0evt_all'] = H1('h_t0evt_all',
                            'Per-event T_{0} (all KLM);T_{0} [ns]', 800, -100, 100)
        h['final_source'] = H1I('h_final_source', 'Final KLM source;;events', 7, 0.5, 7.5)
        for ib, lab in enumerate(['B only', 'E only', 'R only',
                                  'B+E', 'B+R', 'E+R', 'B+E+R'], 1):
            h['final_source'].GetXaxis().SetBinLabel(ib, lab)

        # --- diagnostics/ ---
        f.mkdir('diagnostics').cd()
        h['nhits_B'] = H1I('h_nhits_pertrk_bklm_scint',
                           'Hits per track (BKLM Scint);N_{hits}', 50, 0, 50)
        h['nhits_E'] = H1I('h_nhits_pertrk_eklm_scint',
                           'Hits per track (EKLM Scint);N_{hits}', 50, 0, 50)
        h['sem_B'] = H1('h_sem_pertrk_bklm_scint',
                        'SEM per track (BKLM Scint);SEM [ns]', 200, 0.0, 10.0)
        h['sem_E'] = H1('h_sem_pertrk_eklm_scint',
                        'SEM per track (EKLM Scint);SEM [ns]', 200, 0.0, 10.0)
        h['digitQ_B'] = H1('h_digitQ_bklm_scint',
                           'KLMDigit charge (BKLM Scint);ADC (a.u.)', 100, 0, 800)
        h['digitQ_E'] = H1('h_digitQ_eklm_scint',
                           'KLMDigit charge (EKLM Scint);ADC (a.u.)', 100, 0, 800)
        h['sample_type'] = H1I('h_sample_type', 'Sample type;;events', 2, 0.5, 2.5)
        h['sample_type'].GetXaxis().SetBinLabel(1, 'Data')
        h['sample_type'].GetXaxis().SetBinLabel(2, 'MC')

        # Timing components per detector type
        _timing_cfg = {
            'B': ('BKLM Scint', -5000, -4000, -5000, -4000),
            'R': ('BKLM RPC',    -800,   -500,  -800,  -500),
            'E': ('EKLM Scint', -5000, -4000, -5000, -4000),
        }
        for key, (label, r0, r1, rc0, rc1) in _timing_cfg.items():
            h[f'Trec_{key}'] = H1(f'h_Trec_{key.lower()}',
                                  f'T_{{rec}} ({label});time [ns]', 800, r0, r1)
            h[f'Tcable_{key}'] = H1(f'h_Tcable_{key.lower()}',
                                    f'T_{{cable}} ({label});time [ns]', 800, rc0, rc1)
            h[f'Tprop_{key}'] = H1(f'h_Tprop_{key.lower()}',
                                   f'T_{{prop}} ({label});time [ns]', 800, -50, 50)
            h[f'Tfly_{key}'] = H1(f'h_Tfly_{key.lower()}',
                                  f'T_{{fly}} ({label});time [ns]', 800, -100, 100)

        # --- dimuon/ ---
        f.mkdir('dimuon').cd()
        h['dimuon_all'] = H1('h_dimuon_all',
                             'Dimuon #DeltaT_{0} (all KLM);T_{0}(#mu^{+})-T_{0}(#mu^{-}) [ns]',
                             400, -50, 50)
        h['dimuon_B'] = H1('h_dimuon_bklm_scint',
                           'Dimuon #DeltaT_{0} (BKLM Scint);#DeltaT_{0} [ns]', 400, -50, 50)
        h['dimuon_R'] = H1('h_dimuon_bklm_rpc',
                           'Dimuon #DeltaT_{0} (BKLM RPC);#DeltaT_{0} [ns]', 400, -50, 50)
        h['dimuon_E'] = H1('h_dimuon_eklm_scint',
                           'Dimuon #DeltaT_{0} (EKLM Scint);#DeltaT_{0} [ns]', 400, -50, 50)
        h['dimuon_scint'] = H1('h_dimuon_scint_only',
                               'Dimuon #DeltaT_{0} (Scint only);#DeltaT_{0} [ns]', 400, -50, 50)
        h['dimuon_rpc'] = H1('h_dimuon_with_rpc',
                             'Dimuon #DeltaT_{0} (With RPC);#DeltaT_{0} [ns]', 400, -50, 50)
        h['dimuon_rpcdir'] = H1('h_dimuon_with_rpc_dir',
                                'Dimuon #DeltaT_{0} (With RPC dir);#DeltaT_{0} [ns]', 400, -50, 50)
        # Resolution summary (filled in terminate() from Gaussian fits)
        # 6 categories: BKLM Scint, BKLM RPC, EKLM Scint, Scint Only, With RPC, With RPC Dir
        h['resolution'] = H1('h_per_track_resolution',
                             'Per-Track T_{0} Resolution (#sigma_{#DeltaT0}/#sqrt{2});'
                             'Category;Resolution [ns]', 6, 0.5, 6.5)
        for ib, lab in enumerate(
                ['BKLM Scint', 'BKLM RPC', 'EKLM Scint', 'Scint Only', 'With RPC', 'With RPC Dir'], 1):
            h['resolution'].GetXaxis().SetBinLabel(ib, lab)

        # --- pulls/ ---
        f.mkdir('pulls').cd()
        h['pull_B'] = H1('h_pull_bklm_scint',
                         'Pull (BKLM Scint);(T_{0,i}-T_{0,j})/#sigma', 200, -10, 10)
        h['pull_E'] = H1('h_pull_eklm_scint',
                         'Pull (EKLM Scint);(T_{0,i}-T_{0,j})/#sigma', 200, -10, 10)
        h['pull_Rphi'] = H1('h_pull_rpc_phi',
                            'Pull (BKLM RPC Phi);(T_{0,i}-T_{0,j})/#sigma', 200, -10, 10)
        h['pull_Rz'] = H1('h_pull_rpc_z',
                          'Pull (BKLM RPC Z);(T_{0,i}-T_{0,j})/#sigma', 200, -10, 10)
        h['pull_R'] = H1('h_pull_bklm_rpc',
                         'Pull (BKLM RPC combined);(T_{0,i}-T_{0,j})/#sigma', 200, -10, 10)
        h['pull_BvE'] = H1('h_pull_B_vs_E',
                           'Pull (BKLM Scint vs EKLM Scint);pull', 200, -10, 10)
        h['pull_BvR'] = H1('h_pull_B_vs_R',
                           'Pull (BKLM Scint vs BKLM RPC);pull', 200, -10, 10)
        h['pull_EvR'] = H1('h_pull_E_vs_R',
                           'Pull (EKLM Scint vs BKLM RPC);pull', 200, -10, 10)
        h['pull_smean'] = H1('h_pull_summary_mean',
                             'Pull Mean;Category;#mu', 4, 0.5, 4.5)
        h['pull_swidth'] = H1('h_pull_summary_width',
                              'Pull Sigma;Category;#sigma', 4, 0.5, 4.5)
        for ib, lab in enumerate(['BKLM Scint', 'EKLM Scint', 'RPC Phi', 'RPC Z'], 1):
            h['pull_smean'].GetXaxis().SetBinLabel(ib, lab)
            h['pull_swidth'].GetXaxis().SetBinLabel(ib, lab)

        # Pairwise sector pull histograms
        h['ppw_B'] = self._book_pairwise(
            f, 'pulls/sector/bklm_scint', 'h_pull_B_s{i}_vs_s{j}',
            'BKLM Scint Pull: Sec {i} vs Sec {j};pull',
            _N_BKLM_SECTORS, 200, -10, 10)
        h['ppw_2d_B_mean'] = self._book_2d_summary(
            'h2_pull_bklm_scint_mean', 'Pull Mean (BKLM Scint);Sec 1;Sec 2',
            _N_BKLM_SECTORS)
        h['ppw_2d_B_sigma'] = self._book_2d_summary(
            'h2_pull_bklm_scint_sigma', 'Pull Sigma (BKLM Scint);Sec 1;Sec 2',
            _N_BKLM_SECTORS)

        h['ppw_E'] = self._book_pairwise(
            f, 'pulls/sector/eklm_scint_fwd_vs_bwd',
            'h_pull_E_fwd{i}_vs_bwd{j}',
            'EKLM Pull: Fwd Sec {i} vs Bwd Sec {j};pull',
            _N_EKLM_SECTORS, 200, -10, 10, offset=1)
        h['ppw_2d_E_mean'] = self._book_2d_summary(
            'h2_pull_eklm_fwd_bwd_mean', 'Pull Mean (EKLM Fwd vs Bwd);Fwd Sec;Bwd Sec',
            _N_EKLM_SECTORS, lo=0.5)
        h['ppw_2d_E_sigma'] = self._book_2d_summary(
            'h2_pull_eklm_fwd_bwd_sigma', 'Pull Sigma (EKLM Fwd vs Bwd);Fwd Sec;Bwd Sec',
            _N_EKLM_SECTORS, lo=0.5)

        h['ppw_Rphi'] = self._book_pairwise(
            f, 'pulls/sector/rpc_phi', 'h_pull_Rphi_s{i}_vs_s{j}',
            'RPC Phi Pull: Sec {i} vs Sec {j};pull',
            _N_BKLM_SECTORS, 200, -10, 10)
        h['ppw_2d_Rphi_mean'] = self._book_2d_summary(
            'h2_pull_rpc_phi_mean', 'Pull Mean (RPC Phi);Sec 1;Sec 2', _N_BKLM_SECTORS)
        h['ppw_2d_Rphi_sigma'] = self._book_2d_summary(
            'h2_pull_rpc_phi_sigma', 'Pull Sigma (RPC Phi);Sec 1;Sec 2', _N_BKLM_SECTORS)

        h['ppw_Rz'] = self._book_pairwise(
            f, 'pulls/sector/rpc_z', 'h_pull_Rz_s{i}_vs_s{j}',
            'RPC Z Pull: Sec {i} vs Sec {j};pull',
            _N_BKLM_SECTORS, 200, -10, 10)
        h['ppw_2d_Rz_mean'] = self._book_2d_summary(
            'h2_pull_rpc_z_mean', 'Pull Mean (RPC Z);Sec 1;Sec 2', _N_BKLM_SECTORS)
        h['ppw_2d_Rz_sigma'] = self._book_2d_summary(
            'h2_pull_rpc_z_sigma', 'Pull Sigma (RPC Z);Sec 1;Sec 2', _N_BKLM_SECTORS)

        # --- residuals/ ---
        f.mkdir('residuals').cd()
        h['res_B'] = H1('h_residual_bklm_scint',
                        'Residual (BKLM Scint);#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_E'] = H1('h_residual_eklm_scint',
                        'Residual (EKLM Scint);#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_Rphi'] = H1('h_residual_rpc_phi',
                           'Residual (BKLM RPC Phi);#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_Rz'] = H1('h_residual_rpc_z',
                         'Residual (BKLM RPC Z);#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_R'] = H1('h_residual_bklm_rpc',
                        'Residual (BKLM RPC combined);#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_BvE'] = H1('h_residual_B_vs_E',
                          'Residual BKLM Scint - EKLM Scint;#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_BvR'] = H1('h_residual_B_vs_R',
                          'Residual BKLM Scint - BKLM RPC;#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_EvR'] = H1('h_residual_E_vs_R',
                          'Residual EKLM Scint - BKLM RPC;#DeltaT_{0} [ns]', 200, -50, 50)
        h['res_smean'] = H1('h_residual_summary_mean',
                            'Residual Mean;Category;#mu [ns]', 4, 0.5, 4.5)
        h['res_swidth'] = H1('h_residual_summary_width',
                             'Residual Sigma;Category;#sigma [ns]', 4, 0.5, 4.5)
        for ib, lab in enumerate(['BKLM Scint', 'EKLM Scint', 'RPC Phi', 'RPC Z'], 1):
            h['res_smean'].GetXaxis().SetBinLabel(ib, lab)
            h['res_swidth'].GetXaxis().SetBinLabel(ib, lab)

        # Pairwise sector residual histograms (same structure as pulls)
        h['rpw_B'] = self._book_pairwise(
            f, 'residuals/sector/bklm_scint', 'h_residual_B_s{i}_vs_s{j}',
            'BKLM Scint Residual: Sec {i} vs Sec {j};#DeltaT_{0} [ns]',
            _N_BKLM_SECTORS, 200, -50, 50)
        h['rpw_2d_B_mean'] = self._book_2d_summary(
            'h2_residual_bklm_scint_mean', 'Residual Mean (BKLM Scint);Sec 1;Sec 2',
            _N_BKLM_SECTORS)
        h['rpw_2d_B_sigma'] = self._book_2d_summary(
            'h2_residual_bklm_scint_sigma', 'Residual Sigma (BKLM Scint);Sec 1;Sec 2',
            _N_BKLM_SECTORS)

        h['rpw_E'] = self._book_pairwise(
            f, 'residuals/sector/eklm_scint_fwd_vs_bwd',
            'h_residual_E_fwd{i}_vs_bwd{j}',
            'EKLM Residual: Fwd Sec {i} vs Bwd Sec {j};#DeltaT_{0} [ns]',
            _N_EKLM_SECTORS, 200, -50, 50, offset=1)
        h['rpw_2d_E_mean'] = self._book_2d_summary(
            'h2_residual_eklm_fwd_bwd_mean',
            'Residual Mean (EKLM Fwd vs Bwd);Fwd Sec;Bwd Sec',
            _N_EKLM_SECTORS, lo=0.5)
        h['rpw_2d_E_sigma'] = self._book_2d_summary(
            'h2_residual_eklm_fwd_bwd_sigma',
            'Residual Sigma (EKLM Fwd vs Bwd);Fwd Sec;Bwd Sec',
            _N_EKLM_SECTORS, lo=0.5)

        h['rpw_Rphi'] = self._book_pairwise(
            f, 'residuals/sector/rpc_phi', 'h_residual_Rphi_s{i}_vs_s{j}',
            'RPC Phi Residual: Sec {i} vs Sec {j};#DeltaT_{0} [ns]',
            _N_BKLM_SECTORS, 200, -50, 50)
        h['rpw_2d_Rphi_mean'] = self._book_2d_summary(
            'h2_residual_rpc_phi_mean', 'Residual Mean (RPC Phi);Sec 1;Sec 2',
            _N_BKLM_SECTORS)
        h['rpw_2d_Rphi_sigma'] = self._book_2d_summary(
            'h2_residual_rpc_phi_sigma', 'Residual Sigma (RPC Phi);Sec 1;Sec 2',
            _N_BKLM_SECTORS)

        h['rpw_Rz'] = self._book_pairwise(
            f, 'residuals/sector/rpc_z', 'h_residual_Rz_s{i}_vs_s{j}',
            'RPC Z Residual: Sec {i} vs Sec {j};#DeltaT_{0} [ns]',
            _N_BKLM_SECTORS, 200, -50, 50)
        h['rpw_2d_Rz_mean'] = self._book_2d_summary(
            'h2_residual_rpc_z_mean', 'Residual Mean (RPC Z);Sec 1;Sec 2',
            _N_BKLM_SECTORS)
        h['rpw_2d_Rz_sigma'] = self._book_2d_summary(
            'h2_residual_rpc_z_sigma', 'Residual Sigma (RPC Z);Sec 1;Sec 2',
            _N_BKLM_SECTORS)

        # --- cross_detector/ 4x4 matrix ---
        f.mkdir('cross_detector').cd()
        _reg_names = ['EKLM_Bwd', 'EKLM_Fwd', 'BKLM_RPC', 'BKLM_Scint']
        _reg_labels = ['EKLM Backward', 'EKLM Forward', 'BKLM RPC', 'BKLM Scint']
        h['dt0'] = {}
        for ri in range(_N_REGIONS):
            h['dt0'][ri] = {}
            for rj in range(_N_REGIONS):
                h['dt0'][ri][rj] = H1(
                    f'h_deltaT0_{_reg_names[ri]}_vs_{_reg_names[rj]}',
                    f'#DeltaT_{{0}}: {_reg_labels[ri]} vs {_reg_labels[rj]};'
                    f'#DeltaT_{{0}} [ns]', 200, -50, 50)
        h['dt0_2d_mean'] = TH2D('h2_deltaT0_mean',
                                '#DeltaT_{0} Mean by Region;Region 1;Region 2',
                                _N_REGIONS, -0.5, _N_REGIONS - 0.5,
                                _N_REGIONS, -0.5, _N_REGIONS - 0.5)
        h['dt0_2d_sigma'] = TH2D('h2_deltaT0_sigma',
                                 '#DeltaT_{0} Sigma by Region;Region 1;Region 2',
                                 _N_REGIONS, -0.5, _N_REGIONS - 0.5,
                                 _N_REGIONS, -0.5, _N_REGIONS - 0.5)
        h['dt0_2d_entries'] = TH2D('h2_deltaT0_entries',
                                   '#DeltaT_{0} Entries by Region;Region 1;Region 2',
                                   _N_REGIONS, -0.5, _N_REGIONS - 0.5,
                                   _N_REGIONS, -0.5, _N_REGIONS - 0.5)
        for ri, lab in enumerate(_reg_labels, 1):
            for hh in (h['dt0_2d_mean'], h['dt0_2d_sigma'], h['dt0_2d_entries']):
                hh.GetXaxis().SetBinLabel(ri, lab)
                hh.GetYaxis().SetBinLabel(ri, lab)

        # --- final/ ---
        f.mkdir('final').cd()
        h['final_scint'] = H1('h_t0evt_final_scint_only',
                              'Final KLM T_{0} (Scint only);T_{0} [ns]', 800, -100, 100)
        h['final_rpc'] = H1('h_t0evt_final_with_rpc',
                            'Final KLM T_{0} (Scint+RPC);T_{0} [ns]', 800, -100, 100)
        h['final_rpcdir'] = H1('h_t0evt_final_with_rpc_dir',
                               'Final KLM T_{0} (Scint+RPC dir);T_{0} [ns]', 800, -100, 100)

        # Prevent ROOT garbage-collecting objects not owned by directories
        self._prevent_gc()
        f.cd()

    def _book_pairwise(self, rootfile, dirpath, name_tmpl, title_tmpl,
                       n, nb, lo, hi, offset=0):
        """
        Book an n×n grid of TH1D inside *dirpath* and return as a dict-of-dict.

        *offset* shifts sector indices in name/title (use 1 for EKLM sectors 1–4).
        """
        rootfile.mkdir(dirpath).cd()
        grid = {}
        for i in range(n):
            grid[i] = {}
            for j in range(n):
                si, sj = i + offset, j + offset
                grid[i][j] = TH1D(
                    name_tmpl.format(i=si, j=sj),
                    title_tmpl.format(i=si, j=sj),
                    nb, lo, hi)
        return grid

    def _book_2d_summary(self, name, title, n, lo=-0.5):
        """Book a TH2D for pairwise Gaussian-fit summaries."""
        hi = n + lo
        return TH2D(name, title, n, lo, hi, n, lo, hi)

    def _prevent_gc(self):
        """Call ROOT.SetOwnership(False) on all booked histograms."""
        def _traverse(obj):
            if isinstance(obj, dict):
                for v in obj.values():
                    _traverse(v)
            elif hasattr(obj, 'IsA'):  # it's a ROOT object
                ROOT.SetOwnership(obj, False)
        for v in self._h.values():
            _traverse(v)

    # ------------------------------------------------------------------
    # ExtHit map building (mirrors collectExtrapolatedHits in C++)
    # ------------------------------------------------------------------

    def _build_ext_maps(self, track):
        """
        Build channel→ExtHit maps for *track*.

        Returns
        -------
        scint_map : dict  channel_key → (entry_ExtHit, exit_ExtHit)
        rpc_map   : dict  module_key  → (entry_ExtHit, exit_ExtHit)

        where entry = minimum TOF, exit = maximum TOF among all ExtHits
        with the same key.  This mirrors the C++ matchExt() logic.
        """
        scint_raw = {}   # key → list of ExtHit
        rpc_raw = {}

        muid = track.getRelatedTo('KLMMuidLikelihood')
        elem = self._elem_num

        for ext in track.getRelationsTo('ExtHit'):
            if ext.getStatus() != Belle2.EXT_EXIT:
                continue

            det = ext.getDetectorID()
            is_bklm = (det == Belle2.Const.EDetector.BKLM)
            is_eklm = (det == Belle2.Const.EDetector.EKLM)
            if not is_bklm and not is_eklm:
                continue

            copy_id = ext.getCopyID()

            if is_bklm:
                # Decode BKLM channel number → element numbers
                fwd = ROOT.Long(0)
                sec = ROOT.Long(0)
                lay = ROOT.Long(0)
                pla = ROOT.Long(0)
                strip = ROOT.Long(0)
                Belle2.BKLMElementNumbers.channelNumberToElementNumbers(
                    copy_id, fwd, sec, lay, pla, strip)
                fwd, sec, lay, pla, strip = (int(fwd), int(sec),
                                             int(lay), int(pla), int(strip))

                # Check layer was crossed
                if muid:
                    if not muid.isExtrapolatedBarrelLayerCrossed(lay - 1):
                        continue

                is_rpc = (lay >= Belle2.BKLMElementNumbers.c_FirstRPCLayer)
                if is_rpc:
                    key = elem.moduleNumber(
                        Belle2.KLMElementNumbers.c_BKLM, fwd, sec, lay)
                    rpc_raw.setdefault(key, []).append(ext)
                else:
                    key = elem.channelNumber(
                        Belle2.KLMElementNumbers.c_BKLM, fwd, sec, lay, pla, strip)
                    scint_raw.setdefault(key, []).append(ext)

            else:  # is_eklm
                # Decode EKLM strip number → element numbers
                fwd = ROOT.Long(0)
                lay = ROOT.Long(0)
                sec = ROOT.Long(0)
                pla = ROOT.Long(0)
                strip = ROOT.Long(0)
                Belle2.EKLMElementNumbers.Instance().stripNumberToElementNumbers(
                    copy_id, fwd, lay, sec, pla, strip)
                fwd, lay, sec, pla, strip = (int(fwd), int(lay),
                                             int(sec), int(pla), int(strip))

                if muid:
                    if not muid.isExtrapolatedEndcapLayerCrossed(lay - 1):
                        continue

                key = elem.channelNumber(
                    Belle2.KLMElementNumbers.c_EKLM, fwd, sec, lay, pla, strip)
                scint_raw.setdefault(key, []).append(ext)

        # Reduce each list to (entry, exit) pair by TOF
        def _entry_exit(exts):
            en = min(exts, key=lambda e: e.getTOF())
            ex = max(exts, key=lambda e: e.getTOF())
            return en, ex

        scint_map = {k: _entry_exit(v) for k, v in scint_raw.items()}
        rpc_map = {k: _entry_exit(v) for k, v in rpc_raw.items()}
        return scint_map, rpc_map

    # ------------------------------------------------------------------
    # Per-subdetector accumulators (full correction chain)
    # ------------------------------------------------------------------

    def _accumulate_eklm(self, track, scint_map):
        """
        Accumulate EKLM scintillator hits for *track*.

        Returns
        -------
        results : list of dict, each with:
            t0_est, Trec, Tcable, Tprop, Tfly, section, sector
        """
        results = []
        cable_db = self._db_cable_delay
        status_db = self._db_channel_status
        delay = self._delay_eklm

        for hit2d in track.getRelationsTo('KLMHit2d'):
            if hit2d.getSubdetector() != Belle2.KLMElementNumbers.c_EKLM:
                continue

            for digit in hit2d.getRelationsTo('KLMDigit'):
                if not digit.isGood():
                    continue

                cid = digit.getUniqueChannelID()
                if (status_db.isValid() and
                        status_db.obj().getChannelStatus(cid) !=
                        Belle2.KLMChannelStatus.c_Normal):
                    continue

                # ADC charge fill (before cut)
                charge = digit.getCharge()
                if self._h['digitQ_E']:
                    self._h['digitQ_E'].Fill(charge)

                # ADC cut
                if not (self._adc_eklm_min <= charge <= self._adc_eklm_max):
                    continue

                # Match to ExtHit pair
                pair = scint_map.get(cid)
                if pair is None:
                    continue
                entry_ext, exit_ext = pair

                Tfly = 0.5 * (entry_ext.getTOF() + exit_ext.getTOF())

                # Propagation distance for EKLM strip
                # strip_length is in CLHEP mm; convert to basf2 cm
                strip_len_cm = (self._geo_eklm.getStripLength(digit.getStrip())
                                * _MM_TO_BASF2)

                # Global hit position in CLHEP mm
                pos = 0.5 * (entry_ext.getPosition() + exit_ext.getPosition())
                hit_global = ROOT.HepGeom.Point3Dd(
                    pos.X() * _BASF2_TO_MM,
                    pos.Y() * _BASF2_TO_MM,
                    pos.Z() * _BASF2_TO_MM)

                # Apply strip local transform to get local x
                tr = self._transform_eklm.getStripGlobalToLocal(digit)
                hit_local = tr * hit_global
                # hit_local.x() is in CLHEP mm → convert to cm
                local_x_cm = hit_local.x() * _MM_TO_BASF2

                dist_cm = 0.5 * strip_len_cm - local_x_cm

                # Full correction chain
                Trec = digit.getTime()
                Tcable = (cable_db.obj().getTimeDelay(cid)
                          if cable_db.isValid() else 0.0)
                Tprop = dist_cm * delay
                t0_est = Trec - Tcable - Tprop - Tfly

                if not math.isfinite(t0_est):
                    continue

                # EKLM section: 1=bwd, 2=fwd → convert to 0-indexed for matching
                results.append({
                    't0_est': t0_est,
                    'Trec': Trec, 'Tcable': Tcable,
                    'Tprop': Tprop, 'Tfly': Tfly,
                    'section': digit.getSection(),  # 1=bwd, 2=fwd
                    'sector': digit.getSector(),     # 1-4
                })

        return results

    def _accumulate_bklm_scint(self, track, scint_map):
        """
        Accumulate BKLM scintillator hits for *track*.

        Returns
        -------
        results : list of dict (same fields as _accumulate_eklm, plus
                  'sector' as BKLM sector 1-8)
        """
        results = []
        cable_db = self._db_cable_delay
        status_db = self._db_channel_status
        delay = self._delay_bklm

        for hit2d in track.getRelationsTo('KLMHit2d'):
            if hit2d.getSubdetector() != Belle2.KLMElementNumbers.c_BKLM:
                continue
            if hit2d.inRPC():
                continue
            if hit2d.isOutOfTime():
                continue

            mod = self._geo_bklm.findModule(
                hit2d.getSection(), hit2d.getSector(), hit2d.getLayer())
            pos2d = hit2d.getPosition()

            for b1d in hit2d.getRelationsTo('BKLMHit1d'):
                is_phi = b1d.isPhiReadout()

                for digit in b1d.getRelationsTo('KLMDigit'):
                    if digit.inRPC() or not digit.isGood():
                        continue

                    cid = digit.getUniqueChannelID()
                    if (status_db.isValid() and
                            status_db.obj().getChannelStatus(cid) !=
                            Belle2.KLMChannelStatus.c_Normal):
                        continue

                    charge = digit.getCharge()
                    if self._h['digitQ_B']:
                        self._h['digitQ_B'].Fill(charge)

                    if not (self._adc_bklm_min <= charge <= self._adc_bklm_max):
                        continue

                    pair = scint_map.get(cid)
                    if pair is None:
                        continue
                    entry_ext, exit_ext = pair

                    Tfly = 0.5 * (entry_ext.getTOF() + exit_ext.getTOF())
                    pos_ext = 0.5 * (entry_ext.getPosition() + exit_ext.getPosition())

                    # Gate in local coordinates (position consistency check)
                    loc_ext = mod.globalToLocal(
                        ROOT.CLHEP.Hep3Vector(pos_ext.X(), pos_ext.Y(), pos_ext.Z()),
                        True)
                    loc_hit = mod.globalToLocal(
                        ROOT.CLHEP.Hep3Vector(pos2d.X(), pos2d.Y(), pos2d.Z()),
                        True)
                    diff = loc_ext - loc_hit
                    if (abs(diff.z()) > mod.getZStripWidth() or
                            abs(diff.y()) > mod.getPhiStripWidth()):
                        continue

                    # Propagation distance (in cm, same units as delay constant)
                    dist_cm = mod.getPropagationDistance(loc_ext, digit.getStrip(), is_phi)

                    Trec = digit.getTime()
                    Tcable = (cable_db.obj().getTimeDelay(cid)
                              if cable_db.isValid() else 0.0)
                    Tprop = dist_cm * delay
                    t0_est = Trec - Tcable - Tprop - Tfly

                    if not math.isfinite(t0_est):
                        continue

                    results.append({
                        't0_est': t0_est,
                        'Trec': Trec, 'Tcable': Tcable,
                        'Tprop': Tprop, 'Tfly': Tfly,
                        'section': hit2d.getSection(),  # 0=bwd,1=fwd
                        'sector': hit2d.getSector(),    # 1-8
                    })

        return results

    def _accumulate_bklm_rpc(self, track, rpc_map, accept_phi):
        """
        Accumulate BKLM RPC hits for *track*, filtered by readout direction.

        Parameters
        ----------
        accept_phi : bool
            True  → accumulate phi-plane digits only.
            False → accumulate z-plane digits only.

        Returns
        -------
        results : list of dict (same fields as _accumulate_eklm)
        """
        results = []
        cable_db = self._db_cable_delay
        status_db = self._db_channel_status
        delay = self._delay_rpc_phi if accept_phi else self._delay_rpc_z

        for hit2d in track.getRelationsTo('KLMHit2d'):
            if hit2d.getSubdetector() != Belle2.KLMElementNumbers.c_BKLM:
                continue
            if not hit2d.inRPC():
                continue
            if hit2d.isOutOfTime():
                continue

            mod = self._geo_bklm.findModule(
                hit2d.getSection(), hit2d.getSector(), hit2d.getLayer())
            pos2d = hit2d.getPosition()

            for b1d in hit2d.getRelationsTo('BKLMHit1d'):
                is_phi = b1d.isPhiReadout()
                if accept_phi and not is_phi:
                    continue
                if not accept_phi and is_phi:
                    continue

                for digit in b1d.getRelationsTo('KLMDigit'):
                    if not digit.inRPC():
                        continue
                    if not digit.isGood():
                        continue

                    cid = digit.getUniqueChannelID()
                    if (status_db.isValid() and
                            status_db.obj().getChannelStatus(cid) !=
                            Belle2.KLMChannelStatus.c_Normal):
                        continue

                    # RPC: matched by module key
                    module_key = self._elem_num.moduleNumber(
                        digit.getSubdetector(),
                        digit.getSection(), digit.getSector(), digit.getLayer())
                    pair = rpc_map.get(module_key)
                    if pair is None:
                        continue
                    entry_ext, exit_ext = pair

                    Tfly = 0.5 * (entry_ext.getTOF() + exit_ext.getTOF())
                    pos_ext = 0.5 * (entry_ext.getPosition() + exit_ext.getPosition())

                    loc_ext = mod.globalToLocal(
                        ROOT.CLHEP.Hep3Vector(pos_ext.X(), pos_ext.Y(), pos_ext.Z()),
                        True)
                    loc_hit = mod.globalToLocal(
                        ROOT.CLHEP.Hep3Vector(pos2d.X(), pos2d.Y(), pos2d.Z()),
                        True)
                    diff = loc_ext - loc_hit
                    if (abs(diff.z()) > mod.getZStripWidth() or
                            abs(diff.y()) > mod.getPhiStripWidth()):
                        continue

                    # RPC propagation distance (Hep3Vector, pick y or z component)
                    propa_v = mod.getPropagationDistance(loc_ext)
                    dist_cm = propa_v.y() if is_phi else propa_v.z()

                    Trec = digit.getTime()
                    Tcable = (cable_db.obj().getTimeDelay(cid)
                              if cable_db.isValid() else 0.0)
                    Tprop = dist_cm * delay
                    t0_est = Trec - Tcable - Tprop - Tfly

                    if not math.isfinite(t0_est):
                        continue

                    results.append({
                        't0_est': t0_est,
                        'Trec': Trec, 'Tcable': Tcable,
                        'Tprop': Tprop, 'Tfly': Tfly,
                        'section': hit2d.getSection(),
                        'sector': hit2d.getSector(),
                    })

        return results

    # ------------------------------------------------------------------
    # Event
    # ------------------------------------------------------------------

    def event(self):
        """Read muon tracks, compute per-track T0, fill all histograms."""
        h = self._h

        # Sample type: check for MCInitialParticles
        mc_ip = Belle2.PyStoreObj('MCInitialParticles')
        is_mc = mc_ip.isValid()
        if h.get('sample_type'):
            h['sample_type'].Fill(2 if is_mc else 1)

        muon_list = Belle2.PyStoreObj(self._muon_list_name)
        if not muon_list.isValid():
            return
        n_particles = muon_list.obj().getListSize()
        if n_particles == 0:
            return

        # Per-track results keyed by category:
        # Each entry is a dict:
        #   mu, sem, charge, n_hits, dom_sector, dom_section
        trk_B, trk_E, trk_R, trk_Rphi, trk_Rz = [], [], [], [], []

        for i in range(n_particles):
            particle = muon_list.obj().getParticle(i)
            if not particle:
                continue
            track = particle.getTrack()
            if not track:
                continue
            charge = int(particle.getCharge())

            # Build ExtHit maps for this track
            scint_map, rpc_map = self._build_ext_maps(track)

            # Accumulate per-subdetector
            hits_E = self._accumulate_eklm(track, scint_map)
            hits_B = self._accumulate_bklm_scint(track, scint_map)
            hits_Rphi = self._accumulate_bklm_rpc(track, rpc_map, accept_phi=True)
            hits_Rz = self._accumulate_bklm_rpc(track, rpc_map, accept_phi=False)

            # Fill timing-component diagnostics
            for det_key, hits in (('E', hits_E), ('B', hits_B),
                                  ('R', hits_Rphi + hits_Rz)):
                for hd in hits:
                    for comp in ('Trec', 'Tcable', 'Tprop', 'Tfly'):
                        hh = h.get(f'{comp}_{det_key}')
                        if hh:
                            hh.Fill(hd[comp])

            # Per-track T0 from each category
            def _trk_result(hits, h_nhits, h_sem):
                """Compute (mu, sem, dom_sector, dom_section) for a hit list."""
                if not hits:
                    return None
                vals = [hd['t0_est'] for hd in hits]
                mu, sem = _mu_sem(vals)
                if mu is None or not math.isfinite(mu):
                    return None
                n = len(hits)
                dom_sec = int(round(sum(hd['sector'] for hd in hits) / n))
                dom_section = int(round(sum(hd['section'] for hd in hits) / n))
                if h_nhits:
                    h_nhits.Fill(n)
                if h_sem and sem is not None:
                    h_sem.Fill(sem)
                return mu, (sem if sem is not None else 0.0), dom_sec, dom_section

            res_B = _trk_result(hits_B, h.get('nhits_B'), h.get('sem_B'))
            res_E = _trk_result(hits_E, h.get('nhits_E'), h.get('sem_E'))
            res_Rphi = _trk_result(hits_Rphi, None, None)
            res_Rz = _trk_result(hits_Rz, None, None)

            # Combined RPC (phi + z)
            hits_R = hits_Rphi + hits_Rz
            res_R = _trk_result(hits_R, None, None)

            if res_B:
                mu, se, sec, section = res_B
                if h.get('t0trk_B'):
                    h['t0trk_B'].Fill(mu)
                trk_B.append({'mu': mu, 'sem': se, 'charge': charge,
                              'sector': sec, 'section': section})

            if res_E:
                mu, se, sec, section = res_E
                if h.get('t0trk_E'):
                    h['t0trk_E'].Fill(mu)
                trk_E.append({'mu': mu, 'sem': se, 'charge': charge,
                              'sector': sec, 'section': section})

            if res_R:
                mu, se, sec, section = res_R
                if h.get('t0trk_R'):
                    h['t0trk_R'].Fill(mu)
                trk_R.append({'mu': mu, 'sem': se, 'charge': charge,
                              'sector': sec, 'section': section})

            if res_Rphi:
                mu, se, sec, section = res_Rphi
                trk_Rphi.append({'mu': mu, 'sem': se, 'charge': charge,
                                 'sector': sec, 'section': section})

            if res_Rz:
                mu, se, sec, section = res_Rz
                trk_Rz.append({'mu': mu, 'sem': se, 'charge': charge,
                               'sector': sec, 'section': section})

        if not any([trk_B, trk_E, trk_R]):
            return

        # --- Per-event T0 (track average per category) ---
        def _evt_t0(trk_list):
            pairs = [(t['mu'], t['sem']) for t in trk_list]
            return _weighted_mean(pairs)

        muB, seB = _evt_t0(trk_B)
        muE, seE = _evt_t0(trk_E)
        muR, seR = _evt_t0(trk_R)
        muRphi, seRphi = _evt_t0(trk_Rphi)
        muRz, seRz = _evt_t0(trk_Rz)

        def _safe_fill(key, val):
            hh = h.get(key)
            if hh and val is not None and math.isfinite(val):
                hh.Fill(val)

        _safe_fill('t0evt_B', muB)
        _safe_fill('t0evt_E', muE)
        _safe_fill('t0evt_R', muR)

        # Overall event T0 from all categories
        all_parts = [(mu, se) for mu, se in ((muB, seB), (muE, seE), (muR, seR))
                     if mu is not None and math.isfinite(mu)]
        muAll, _ = _weighted_mean(all_parts)
        _safe_fill('t0evt_all', muAll)

        # Final source audit
        useB = muB is not None and math.isfinite(muB)
        useE = muE is not None and math.isfinite(muE)
        useR = muR is not None and math.isfinite(muR)
        if any((useB, useE, useR)):
            src = (useB, useE, useR)
            src_map = {
                (True,  False, False): 1,
                (False, True,  False): 2,
                (False, False, True):  3,
                (True,  True,  False): 4,
                (True,  False, True):  5,
                (False, True,  True):  6,
                (True,  True,  True):  7,
            }
            sb = src_map.get(src)
            if sb and h.get('final_source'):
                h['final_source'].Fill(sb)

        # --- Final T0 combinations ---
        def _combine(parts):
            """Inverse-variance combine non-None (mu, se) pairs."""
            valid = [(mu, se) for mu, se in parts
                     if mu is not None and math.isfinite(mu)]
            return _weighted_mean(valid)

        t0_scint, _ = _combine([(muB, seB), (muE, seE)])
        t0_rpc, _ = _combine([(muB, seB), (muE, seE), (muR, seR)])
        t0_rpcdir, _ = _combine([(muB, seB), (muE, seE),
                                 (muRphi, seRphi), (muRz, seRz)])
        _safe_fill('final_scint', t0_scint)
        _safe_fill('final_rpc', t0_rpc)
        _safe_fill('final_rpcdir', t0_rpcdir)

        # --- Same-detector pair histograms (pulls and residuals) ---
        self._fill_pairs(trk_B, h.get('pull_B'), h.get('res_B'),
                         h.get('ppw_B'), h.get('rpw_B'), bklm=True)
        self._fill_pairs(trk_E, h.get('pull_E'), h.get('res_E'),
                         h.get('ppw_E'), h.get('rpw_E'), bklm=False)
        self._fill_pairs(trk_Rphi, h.get('pull_Rphi'), h.get('res_Rphi'),
                         h.get('ppw_Rphi'), h.get('rpw_Rphi'), bklm=True)
        self._fill_pairs(trk_Rz, h.get('pull_Rz'), h.get('res_Rz'),
                         h.get('ppw_Rz'), h.get('rpw_Rz'), bklm=True)
        self._fill_pairs(trk_R, h.get('pull_R'), h.get('res_R'),
                         None, None, bklm=True)

        # --- Cross-detector pulls (event-level) ---
        def _cross_pull(muX, seX, muY, seY, h_pull, h_res):
            if muX is None or muY is None:
                return
            if not (math.isfinite(muX) and math.isfinite(muY)):
                return
            dt = muX - muY
            if h_res and math.isfinite(dt):
                h_res.Fill(dt)
            if (seX is not None and seY is not None and
                    seX > 0 and seY > 0):
                denom2 = seX * seX + seY * seY
                pull = dt / math.sqrt(denom2)
                if h_pull and math.isfinite(pull):
                    h_pull.Fill(pull)

        _cross_pull(muB, seB, muE, seE, h.get('pull_BvE'), h.get('res_BvE'))
        _cross_pull(muB, seB, muR, seR, h.get('pull_BvR'), h.get('res_BvR'))
        _cross_pull(muE, seE, muR, seR, h.get('pull_EvR'), h.get('res_EvR'))

        # --- Cross-detector 4x4 ΔT0 matrix ---
        # Map detector regions to per-event T0 values
        # Regions: 0=EKLM Bwd, 1=EKLM Fwd, 2=BKLM RPC, 3=BKLM Scint
        def _eklm_region_t0(trk_list, target_section):
            """Per-event T0 from tracks in a specific EKLM section."""
            filtered = [t for t in trk_list if t['section'] == target_section]
            if not filtered:
                return None, None
            return _weighted_mean([(t['mu'], t['sem']) for t in filtered])

        reg_t0 = {
            _REG_EKLM_BWD: _eklm_region_t0(trk_E, _EKLM_SECTION_BWD),
            _REG_EKLM_FWD: _eklm_region_t0(trk_E, _EKLM_SECTION_FWD),
            _REG_BKLM_RPC:   (muR, seR),
            _REG_BKLM_SCINT: (muB, seB),
        }
        dt0_h = h.get('dt0', {})
        for ri in range(_N_REGIONS):
            mu_i, se_i = reg_t0[ri]
            if mu_i is None or not math.isfinite(mu_i):
                continue
            for rj in range(_N_REGIONS):
                mu_j, se_j = reg_t0[rj]
                if mu_j is None or not math.isfinite(mu_j):
                    continue
                dt = mu_i - mu_j
                hh = dt0_h.get(ri, {}).get(rj)
                if hh and math.isfinite(dt):
                    hh.Fill(dt)

        # --- Dimuon ΔT0 ---
        def _dimuon_fill(trk_list, h_hist):
            if not h_hist:
                return
            t0p = t0m = None
            for t in trk_list:
                if t['charge'] > 0:
                    t0p = t['mu']
                elif t['charge'] < 0:
                    t0m = t['mu']
            if (t0p is not None and t0m is not None and
                    math.isfinite(t0p) and math.isfinite(t0m)):
                h_hist.Fill(t0p - t0m)

        _dimuon_fill(trk_B, h.get('dimuon_B'))
        _dimuon_fill(trk_E, h.get('dimuon_E'))
        _dimuon_fill(trk_R, h.get('dimuon_R'))

        # All KLM combined (use first available T0 per charge sign: B > E > R)
        trk_all_by_charge = {}
        for lst in (trk_B, trk_E, trk_R):
            for t in lst:
                q = t['charge']
                if q not in trk_all_by_charge:
                    trk_all_by_charge[q] = t
        _dimuon_fill(list(trk_all_by_charge.values()), h.get('dimuon_all'))

        # Scint-only combined per track: B+E hits combined
        trk_scint = self._combine_track_categories(trk_B, trk_E)
        _dimuon_fill(trk_scint, h.get('dimuon_scint'))

        # With-RPC combined per track: B+E+R hits combined (RPC as single category)
        trk_all_cats = self._combine_track_categories(trk_B, trk_E, trk_R)
        _dimuon_fill(trk_all_cats, h.get('dimuon_rpc'))

        # With-RPC-dir combined per track: B+E+Rphi+Rz as separate categories
        trk_rpcdir_cats = self._combine_track_categories(trk_B, trk_E, trk_Rphi, trk_Rz)
        _dimuon_fill(trk_rpcdir_cats, h.get('dimuon_rpcdir'))

    # ------------------------------------------------------------------
    # Pair histogram helpers
    # ------------------------------------------------------------------

    def _fill_pairs(self, trk_list, h_pull, h_res, ppw_pull, ppw_res, bklm):
        """
        Fill same-detector pair histograms for one category.

        For BKLM (bklm=True): sector key = sector (0-indexed for 8 sectors).
        For EKLM (bklm=False): sector key = sector-1 (0-indexed for 4 sectors).
        """
        n = len(trk_list)
        for i in range(n):
            ti = trk_list[i]
            if not math.isfinite(ti['mu']):
                continue
            for j in range(i + 1, n):
                tj = trk_list[j]
                if not math.isfinite(tj['mu']):
                    continue
                if self._opp_charges_only and ti['charge'] * tj['charge'] >= 0:
                    continue

                dt = ti['mu'] - tj['mu']
                if not math.isfinite(dt):
                    continue

                if h_res:
                    h_res.Fill(dt)

                si = ti['sem']
                sj = tj['sem']
                denom2 = si * si + sj * sj if (si and sj) else 0.0
                if denom2 > 0.0:
                    pull = dt / math.sqrt(denom2)
                    if h_pull and math.isfinite(pull):
                        h_pull.Fill(pull)

                    # Pairwise sector histograms
                    if ppw_pull or ppw_res:
                        if bklm:
                            # BKLM sectors 1-8 → 0-indexed key
                            ki = ti['sector'] - 1
                            kj = tj['sector'] - 1
                            n_max = _N_BKLM_SECTORS
                        else:
                            # EKLM sectors 1-4 → 0-indexed key
                            # Pairwise is Fwd (section=2) vs Bwd (section=1)
                            # Use section to assign which side each track is on
                            ki = ti['sector'] - 1
                            kj = tj['sector'] - 1
                            n_max = _N_EKLM_SECTORS

                        if 0 <= ki < n_max and 0 <= kj < n_max:
                            if ppw_pull:
                                pg = ppw_pull.get(ki, {}).get(kj)
                                if pg and math.isfinite(pull):
                                    pg.Fill(pull)
                            if ppw_res:
                                rg = ppw_res.get(ki, {}).get(kj)
                                if rg and math.isfinite(dt):
                                    rg.Fill(dt)

    def _combine_track_categories(self, *categories):
        """
        Combine multiple per-track category lists into a single list,
        merging by charge (one entry per charge sign, weighted mean of T0).
        """
        by_charge = {}
        for cat in categories:
            for t in cat:
                q = t['charge']
                if q not in by_charge:
                    by_charge[q] = []
                by_charge[q].append((t['mu'], t['sem']))

        result = []
        for q, pairs in by_charge.items():
            mu, se = _weighted_mean(pairs)
            if mu is not None and math.isfinite(mu):
                result.append({'mu': mu, 'sem': se or 0.0, 'charge': q,
                               'sector': -1, 'section': -1})
        return result

    # ------------------------------------------------------------------
    # Summary histogram fitting (called from terminate)
    # ------------------------------------------------------------------

    def _fit_summary_histograms(self):
        """
        Fit Gaussian to pull/residual/dimuon histograms and fill
        summary histograms (mean, sigma per category; 2D pairwise summaries;
        per-track T0 resolution from dimuon ΔT0).
        """
        h = self._h

        # Pull summary (4 categories: B, E, Rphi, Rz)
        for ib, (key, label) in enumerate([('pull_B', 'BKLM Scint'),
                                           ('pull_E', 'EKLM Scint'),
                                           ('pull_Rphi', 'RPC Phi'),
                                           ('pull_Rz', 'RPC Z')], 1):
            mu, muE, sig, sigE = _gauss_fit(h.get(key), -5.0, 5.0)
            if mu is not None:
                h['pull_smean'].SetBinContent(ib, mu)
                h['pull_smean'].SetBinError(ib, muE)
                h['pull_swidth'].SetBinContent(ib, sig)
                h['pull_swidth'].SetBinError(ib, sigE)
                basf2.B2INFO(f'KLMEventT0Validation: {label} pull fit:'
                             f' mean={mu:.3f}±{muE:.3f}  sigma={sig:.3f}±{sigE:.3f}')

        # Residual summary (4 categories)
        for ib, (key, label) in enumerate([('res_B', 'BKLM Scint'),
                                           ('res_E', 'EKLM Scint'),
                                           ('res_Rphi', 'RPC Phi'),
                                           ('res_Rz', 'RPC Z')], 1):
            mu, muE, sig, sigE = _gauss_fit(h.get(key), -25.0, 25.0)
            if mu is not None:
                h['res_smean'].SetBinContent(ib, mu)
                h['res_smean'].SetBinError(ib, muE)
                h['res_swidth'].SetBinContent(ib, sig)
                h['res_swidth'].SetBinError(ib, sigE)

        # Pairwise sector pull summaries
        _pairwise_cfgs = [
            ('ppw_B',    'ppw_2d_B_mean',    'ppw_2d_B_sigma',    _N_BKLM_SECTORS),
            ('ppw_E',    'ppw_2d_E_mean',    'ppw_2d_E_sigma',    _N_EKLM_SECTORS),
            ('ppw_Rphi', 'ppw_2d_Rphi_mean', 'ppw_2d_Rphi_sigma', _N_BKLM_SECTORS),
            ('ppw_Rz',   'ppw_2d_Rz_mean',   'ppw_2d_Rz_sigma',   _N_BKLM_SECTORS),
        ]
        for pw_key, mean_key, sig_key, n in _pairwise_cfgs:
            pw = h.get(pw_key, {})
            h2m = h.get(mean_key)
            h2s = h.get(sig_key)
            for i in range(n):
                for j in range(n):
                    mu, _, sig, _ = _gauss_fit(pw.get(i, {}).get(j), -5.0, 5.0)
                    if mu is not None:
                        if h2m:
                            h2m.SetBinContent(i + 1, j + 1, mu)
                        if h2s:
                            h2s.SetBinContent(i + 1, j + 1, sig)

        # Pairwise sector residual summaries
        _rpairwise_cfgs = [
            ('rpw_B',    'rpw_2d_B_mean',    'rpw_2d_B_sigma',    _N_BKLM_SECTORS),
            ('rpw_E',    'rpw_2d_E_mean',    'rpw_2d_E_sigma',    _N_EKLM_SECTORS),
            ('rpw_Rphi', 'rpw_2d_Rphi_mean', 'rpw_2d_Rphi_sigma', _N_BKLM_SECTORS),
            ('rpw_Rz',   'rpw_2d_Rz_mean',   'rpw_2d_Rz_sigma',   _N_BKLM_SECTORS),
        ]
        for pw_key, mean_key, sig_key, n in _rpairwise_cfgs:
            pw = h.get(pw_key, {})
            h2m = h.get(mean_key)
            h2s = h.get(sig_key)
            for i in range(n):
                for j in range(n):
                    mu, _, sig, _ = _gauss_fit(pw.get(i, {}).get(j), -25.0, 25.0)
                    if mu is not None:
                        if h2m:
                            h2m.SetBinContent(i + 1, j + 1, mu)
                        if h2s:
                            h2s.SetBinContent(i + 1, j + 1, sig)

        # Cross-detector ΔT0 2D summary
        dt0_h = h.get('dt0', {})
        h2m = h.get('dt0_2d_mean')
        h2s = h.get('dt0_2d_sigma')
        h2e = h.get('dt0_2d_entries')
        for ri in range(_N_REGIONS):
            for rj in range(_N_REGIONS):
                hh = dt0_h.get(ri, {}).get(rj)
                if hh is None:
                    continue
                if h2e:
                    h2e.SetBinContent(ri + 1, rj + 1, hh.GetEntries())
                mu, _, sig, _ = _gauss_fit(hh, -25.0, 25.0)
                if mu is not None:
                    if h2m:
                        h2m.SetBinContent(ri + 1, rj + 1, mu)
                    if h2s:
                        h2s.SetBinContent(ri + 1, rj + 1, sig)

        # Per-track T0 resolution from dimuon ΔT0:  σ_track = σ(ΔT0)/√2
        # 6 categories matching resolution histogram bin labels
        h_res = h.get('resolution')
        for ib, dm_key in enumerate(
                ['dimuon_B', 'dimuon_R', 'dimuon_E',
                 'dimuon_scint', 'dimuon_rpc', 'dimuon_rpcdir'], 1):
            _, _, sig, sigE = _gauss_fit(h.get(dm_key), -40.0, 40.0)
            if sig is not None and h_res:
                per_track = sig / math.sqrt(2.0)
                per_track_err = sigE / math.sqrt(2.0)
                h_res.SetBinContent(ib, per_track)
                h_res.SetBinError(ib, per_track_err)
