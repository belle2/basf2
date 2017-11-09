#!/usr/bin/env python
# coding: utf8

"""
SVDSimBase.py
==========
The module contains basic functions for generation and analysis of SVD strip data.

:author: Peter Kvasnicka

Description:
This is a set of utility functions and constant declarations for simulation of SVD strip data
to use in training timing networks or for other purposes. These include wave functions (gamma,
cubic polynomial and beta-prime).

Usage:
Just import the module to use it.
"""

import math
import numpy as np
import pandas as pd
from scipy.stats import norm, uniform

dt = 31.44  # Time interval between APV25 samples
threshold_cut = 3  # Samples with signal-to-noise less than 3 are zero.

wexp_default_tau = 55  # ns


def wexp(t, tau=wexp_default_tau):
    '''Gamma waveform
    wexp(t, tau) = t/tau * exp(1-t/tau) if t > 0 else 0,
    normalized to peak value 1.
    t - nummpy vector of times
    tau - (scalar) waveform decay time
    return: numpy vector of wexp(t, tau) values at times t
    '''
    z = t / tau
    return np.clip(z * np.exp(1.0 - z), 0.0, 100.0)


w3_default_tau = 200  # ns


def w3(t, tau=w3_default_tau):
    ''' Cubic waveform
    w3(t, tau) = 27/4 * t/tau * (1-t/tau)**2 if 0 < t < tau else 0,
    normalized to peak value 1.
    t - numpy vector of times
    tau - (scalar) width of the distribution
    return - numpy vector of w3(t,tau) values at times t
    '''
    z = np.clip(t / tau, 0, 1)
    return 27 / 4 * z * (z - 1) * (z - 1)


bp_default_tau = 250  # ns


def betaprime_wave(t, tau=bp_default_tau):
    ''' Beta-prime waveform
    betaprime_wave(t, tau) = 149.012 * (t/tau)**2 * (1 + t/tau)**10 if t > 0 else 0
    t - numpy vector of times
    tau - (scalar) width of the distribution
    return - numpy vector of betaprime_wave values of betaprime_wave at times t
    '''
    z = np.clip(t / tau, 0, 1000)
    return 149.012 * z**2 / (1 + z)**10


def test3(s, threshold):
    '''Test for 3 consecutive non-zero APV samples
    A six-tuple of APV25 samples is only accepted if it has 3 consecutive samples over threshold.
    s - 6 apv samples
    return - True if there are 3 consecutive samples over threshold, otherwise False.
    '''
    cons_before = np.zeros(len(s))
    for i in range(3):
        cons_before[i:] += (s > threshold)[0:6 - i]
    return np.max(cons_before) >= 3


'''
This is the list of all configurations of over-the-threhsold samples that pass the 3-over-the-threshold-samples test.
'''
residual_configs = [
    [3, 4, 5],
    [2, 3, 4],
    [2, 3, 4, 5],
    [1, 3, 4, 5],
    [1, 2, 3],
    [1, 2, 3, 5],
    [1, 2, 3, 4],
    [1, 2, 3, 4, 5],
    [0, 3, 4, 5],
    [0, 2, 3, 4],
    [0, 2, 3, 4, 5],
    [0, 1, 3, 4, 5],
    [0, 1, 2],
    [0, 1, 2, 5],
    [0, 1, 2, 4],
    [0, 1, 2, 4, 5],
    [0, 1, 2, 3],
    [0, 1, 2, 3, 5],
    [0, 1, 2, 3, 4],
    [0, 1, 2, 3, 4, 5]
]


def gen_signal(ampl, t0, tau, sigma=1, w=betaprime_wave, tau_sigma=0.0):
    '''Generate random sample of 6 APV signals, properly censored.
    The produced data are normalized to sigma = 1 and the given amplitude.
    The procedure is:
    - real_amplitude = sigma * amlitude
    - generate waveform, add noise, and convert to integer values
    - divide signals by sigma
    For large amplitudes, the function adds gaussian noise to generated waveform.
    For amplitudes around and below the threshold, the function randomly selects a
    subset of samples over threshold and generates over-threhold noises from left-censored
    gaussian distributions.
    ampl - amplitude
    t0 - time shift
    tau - decay time or width of the waveform
    sigma - nosie (default 1: we work with S/N rather than raw signals)
    tau_sigma - width of tau jitter
    '''
    # reconstruct amplitude
    gen_amplitude = sigma * ampl
    # threshold for data generration from cdf
    gen_thr = int(sigma * threshold_cut + 1.0 - 1.0e-9) - 0.5
    res = np.zeros(6)
    # Add tau jitter, if desired.
    if tau_sigma > 0:
        tau += np.random.randn() * tau_sigma
    res0 = gen_amplitude * w(np.linspace(-dt - t0, 4 * dt - t0, 6, endpoint=True), tau)
    if test3(res0, threshold_cut * sigma):
        res = (res0 + sigma * np.random.randn(6) + 0.5).astype(int)
        # We just repeat if the configuration doesn't pass (should happen rarely)
        while not test3(res, threshold_cut * sigma):
            res = (res0 + sigma * np.random.randn(6) + 0.5).astype(int)
        res[res < threshold_cut * sigma] = 0
    else:  # low-amp mode
        # calculate probabilities of saamples above threhold
        p_over = 1.0 - norm.cdf((gen_thr - res0) / sigma)
        # calculate probabilities of over-threhold configurations
        pconfs = np.array([np.prod(p_over[conf]) for conf in residual_configs])
        pconfs /= np.sum(pconfs)
        cconfs = np.cumsum(pconfs)
        # select random configuration
        u_conf = uniform.rvs()
        i_conf = 0  # meaning configuration 0
        while u_conf > cconfs[i_conf]:
            i_conf += 1
        u_res = uniform.rvs(0, 1, len(residual_configs[i_conf]))
        res = np.zeros(6)
        res[residual_configs[i_conf]] = res0[residual_configs[i_conf]] + \
            sigma * norm.ppf(1 - u_res * p_over[residual_configs[i_conf]])
        res = (res + 0.5).astype(int)
    return res / sigma

# ==============================================================================


class tau_encoder:
    # ------------------------------------------------------------------------------
    '''
    A simple class to encode and decode tau values for network training
    based on amplitude and tau ranges.
    '''

    def __init__(self, amp_range, tau_range):
        self.amp_min, self.amp_max = amp_range
        self.tau_min, self.tau_max = tau_range
        self.at_ratio = (self.amp_max - self.amp_min) / (self.tau_max - self.tau_min)

    def encode(self, tau):
        return (self.amp_min + self.at_ratio * (tau - self.tau_min))

    def decode(self, etau):
        return (self.tau_min + 1.0 / self.at_ratio * (etau - self.amp_min))


class SampleGenerator:
    # ------------------------------------------------------------------------------
    '''
    This class generates a Pandas dataframe with a random sample of SVD strip signals with specified size and parameters.
    NB:
    1. We generate time bins from quantiles, do we want a regular grid?
    2. Have to think of possible irregular grid.
    '''

    def __init__(self, t0_bounds, tau_bounds, amplitude_bounds, sigma_bounds, tau_sigma, bin_size, wf=betaprime_wave):
        '''
        The constructor takes the following parameters:
        t0_bounds is a tuple, (t0_min, t0_max)
        tau_bounds is a tuple (tau_min, tau_max)
        amplitude_bounds is a tuple (amp_min, amp_max)
        sigma_bounds is a tuple (sigma_min, sigma_max)
        bin_size is the % fraction of t0_min, t0_max interval corresponding to a single output t0 bin.
        '''
        self.t0_min, self.t0_max = t0_bounds
        self.tau_min, self.tau_max = tau_bounds
        self.amp_min, self.amp_max = amplitude_bounds
        self.sigma_min, self.sigma_max = sigma_bounds
        self.tau_coder = tau_encoder(amplitude_bounds, tau_bounds)
        self.tau_sigma = tau_sigma
        self.bin_size = bin_size
        self.wf = wf

    def get_t0_bounds(self):
        '''
        Get t0 bounds of sampling space
        '''
        return (self.t0_min, self.t0_max)

    def get_amp_bounds(self):
        '''
        Get amplitude bounds of sampling space
        '''
        return (self.amp_min, self.amp_max)

    def get_tau_bounds(self):
        '''
        Get tau bounds of sampling space
        '''
        return (self.tau_min, self.tau_max)

    def set_tau_bounds(self, tau_min, tau_max):
        '''
        Set width limits for the simulation.
        '''
        self.tau_min = tau_min
        self.tau_max = tau_max

    def set_tau_sigma(self, tau_sigma):
        '''
        Set width jitter for the simulation.
        '''
        self.tau_sigma = tau_sigma

    def get_tau_sigma(self):
        '''
        Get width jitter for the simulation.
        '''
        return self.tau_sigma

    def get_sigma_bounds(self):
        '''
        Get sigma bounds
        '''
        return (self.sigma_min, self.sigma_max)

    def generate(self, sample_size):
        '''
        Generate sample_size samples.
        '''
        self.n_samples = sample_size
        self.stockdata = pd.DataFrame({
            'test': np.random.uniform(size=self.n_samples),
            't0': np.random.uniform(self.t0_min, self.t0_max, size=self.n_samples),
            'tau': np.random.uniform(self.tau_min, self.tau_max, size=self.n_samples),
            'sigma': np.random.uniform(self.sigma_min, self.sigma_max, size=self.n_samples),
            'amplitude': np.random.uniform(self.amp_min, self.amp_max, size=self.n_samples),
            's1': np.zeros(self.n_samples),
            's2': np.zeros(self.n_samples),
            's3': np.zeros(self.n_samples),
            's4': np.zeros(self.n_samples),
            's5': np.zeros(self.n_samples),
            's6': np.zeros(self.n_samples)
        })
        self.stockdata['normed_tau'] = self.stockdata.apply(lambda row: self.tau_coder.encode(row.tau), axis=1)
        orderedcols = ['test', 'amplitude', 't0', 'tau', 'sigma', 's1', 's2', 's3', 's4', 's5', 's6', 'normed_tau']
        self.stockdata = self.stockdata[orderedcols]
        # This is where the data are generated
        self.stockdata[['s' + str(i) for i in range(1, 7)]] = self.stockdata.apply(lambda row: pd.Series(
            gen_signal(row.amplitude, row.t0, row.tau, row.sigma, tau_sigma=self.tau_sigma, w=self.wf)), axis=1)
        self.t0_bins = np.percentile(self.stockdata.t0, np.arange(0, 101, self.bin_size))
        self.t0_bins[0] = self.t0_bins[0] - 0.1
        self.t0_bins[-1] = self.t0_bins[-1] + 0.1
        self.stockdata['t0_bin'] = np.digitize(self.stockdata.t0, self.t0_bins)
        self.t0_bin_times = self.stockdata['t0'].groupby(self.stockdata.t0_bin).aggregate(np.mean)
        abins = np.arange(self.amp_min, self.amp_max + 1, 1)
        self.stockdata['abin'] = np.digitize(self.stockdata.amplitude, abins)
        return self.stockdata

    def get_t0_array(self):
        '''
        Get array of mean t0's for classifier bins
        '''
        return self.t0_bin_times

    def get_t0_bins(self):
        '''
        Get array of mean t0's for classifier bins
        '''
        return pd.Series(self.t0_bins)


# ==============================================================================
# Tau (scale) conversion and encoding
# ------------------------------------------------------------------------------
'''
Empirical ranges of raw waveform width values from February 2017 testbeam data.
These values are only used for scaling and not critical.
'''
raw_tau_min = 27
raw_tau_max = 45


def tau_hao2real(hao_tau):
    '''
    Convert Hao's raw tau (integral, in latency units) to correct betaprime scale. Includes scaling and fit adjustment.
    '''
    return 3.93 / 0.50305 * hao_tau


# ------------------------------------------------------------------------------

if __name__ == '__main__':
    print('Basic functions for simulation of SVD strip data.\nImport to use.')
