
# coding: utf-8

# ## Generate a data sample for SVDTime Neural Network training
#
# This script generates a toy data sample for neural network training.
# The result is stored as pickle and additional pickles contain parameters
# of training.
#
# TO DO / FIX:
# * We can make the generation faster using C++.
# * If we stored in ROOT, we wouldn't need to keep all data in memory.
#
# Packages required:
# - pandas
# - SVDSiimBase
# - argparse

# In[1]:

import pandas as pd
from svd.SVDSimBase import *
import argparse

# ### Sample generation
#
# Generate a pandas dataframe containing a large number of waveform samples and truth data, and pickle it.
# The data will be used as training and test data.
# Waveform widths (tau), amplitudes and time shifts are sampled uniformly from a large set of feasible values.
# Additionally, waveform widths (tau) are jittered using a normal
# distribution with 5 ns width: that is, the "true" values are slightly
# off. This is to robustify against imprecise knowledge of waveform width
# or shape.

parser = argparse.ArgumentParser(description="Simulation of toy data for training of SVD hit time esitmator")

parser.add_argument(
    '--nsamples',
    dest='n_samples',
    action='store',
    default=1000000,
    type=int,
    help='Size of the sample to generate'
)
parser.add_argument(
    '--low-t0',
    dest='t0_low',
    action='store',
    default=-2.5 * dt,
    type=float,
    help='Lower bound of t0 distribution, ns')
parser.add_argument(
    '--high-t0',
    dest='t0_high',
    action='store',
    default=1.5 * dt,
    type=float,
    help='Upper bound of t0 distribution, ns')
parser.add_argument(
    '--low-tau',
    dest='tau_low',
    action='store',
    default=200,
    type=float,
    help='Lower bound of tau distribution, ns')
parser.add_argument(
    '--high-tau',
    dest='tau_high',
    action='store',
    default=350,
    type=float,
    help='Upper bound of tau distribution, ns')
parser.add_argument(
    '--jitter-tau',
    dest='tau_jitter',
    action='store',
    default=5,
    type=float,
    help='RMS of tau jitter, 0 - no jitter, ns')
parser.add_argument(
    '--low-amp',
    dest='amp_low',
    action='store',
    default=3,
    type=float,
    help='Lower bound of amplitude distribution, S/N units')
parser.add_argument(
    '--high-amp',
    dest='amp_high',
    action='store',
    default=100,
    type=float,
    help='Upper bound of amplitude distribution, S/N units')
parser.add_argument(
    '--low-sigma',
    dest='sigma_low',
    action='store',
    default=1,
    type=float,
    help='Lower bound of sigma distribution, S/N units')
parser.add_argument(
    '--high-sigma',
    dest='sigma_high',
    action='store',
    default=5,
    type=float,
    help='Upper bound of sigma distribution, S/N units')
parser.add_argument(
    '--bin_size',
    dest='bin_size',
    action='store',
    default=3,
    type=float,
    help='Size of t0 PDF bin, ns')

args = parser.parse_args()

generator = SampleGenerator(
    (args.t0_low, args.t0_high),
    (args.tau_low, args.tau_high),
    (args.amp_low, args.amp_high),
    (args.sigma_low, args.sigma_high),
    args.tau_jitter,
    args.bin_size)

print('Generating {0} samples...'.format(args.n_samples))
sample = generator.generate(args.n_samples)

# Create a bin table
timearray = generator.get_t0_array()
timebins = generator.get_t0_bins()
bins = pd.DataFrame({
    'midpoint': timearray,
    'lower': timebins.values[:-1],
    'upper': timebins.values[1:]
})

# Create a table of simulation bounds
bounds = pd.DataFrame({
    'value': np.array(['t0', 'amplitude', 'tau', 'sigma']),
    'sampling': np.array(['uniform', 'uniform', 'uniform', 'uniform']),
    'low': [
        generator.get_t0_bounds()[0],
        generator.get_amp_bounds()[0],
        args.tau_low,
        generator.get_sigma_bounds()[0]
    ],
    'high': [
        generator.get_t0_bounds()[1],
        generator.get_amp_bounds()[1],
        args.tau_high,
        generator.get_sigma_bounds()[1]
    ]
})
orderedcols = ['value', 'sampling', 'low', 'high']
bounds = bounds[orderedcols]

print('Samples created.')

output_name = 'SVDTime_Training{0}_{1}.pkl'

# There will be three trees: sample, bins, bounds.

sample.to_pickle(output_name.format('Sample', args.n_samples))
bins.to_pickle(output_name.format('Bins', args.n_samples))
bounds.to_pickle(output_name.format('Bounds', args.n_samples))

print('Done.\nResults saved to {0}.'.format(output_name.format(
    '{Sample, Bins, Bounds}', args.n_samples)))
