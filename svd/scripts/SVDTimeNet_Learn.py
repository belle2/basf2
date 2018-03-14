
# coding: utf-8

# ## Train and save the SVDTime Neural Network
#
# The SVDTimeNN is a MultilayerPerceptron estimator of
# The truth data in this case are bin numbers in a series of time shift
# bins. The result of such a training is a distribution function for a
# time shift value. From these, it is easy to calculate mean value and
# standard deviation, but also do a range of approximate probabilistic
# calculations.

# ##### Required Python packages
#
# The following python packages are used:
# - math (basic python math functions)
# - numpy (Vectors and matrices for numerics)
# - pandas (Python analogue of Excel tables)
# - scipy (Scientific computing package)
# - scikit-learn (machine learning)
#
# Only sklear2pmml is missing in the current basf2 distribution.
# Install it with
#
# pip3 install --user git+https://github.com/jpmml/sklearn2pmml.git
#
# ##### Other pre-requisites:
#
# A sample of training data, plus binning and bounds information in pickle (*.pkl) files.

import math
import datetime
import pickle
import numpy as np
import pandas as pd
from scipy import stats as stats
from scipy.optimize import minimize_scalar
from sklearn.neural_network import MLPClassifier
from sklearn2pmml import sklearn2pmml, PMMLPipeline
from svd.SVDSimBase import *
from lxml import etree as ET
import argparse

# ### Retrieve training sample

parser = argparse.ArgumentParser(description="Train the SVD hit time estimator")

parser.add_argument(
    '--nsamples',
    dest='n_samples',
    action='store',
    default=1000000,
    type=int,
    help='Global tag to use at central DB in PNNL')

args = parser.parse_args()

pkl_name = 'SVDTime_Training{0}_{1}.pkl'

print('Reading data...')

stockdata = pd.read_pickle(pkl_name.format('Sample', args.n_samples))
bounds = pd.read_pickle(pkl_name.format('Bounds', args.n_samples))
bins = pd.read_pickle(pkl_name.format('Bins', args.n_samples))

timearray = bins['midpoint']
timebins = np.unique(bins[['lower', 'upper']])

print('Done.')

# ### Split the data into training and test samples

test_fraction = 0.2
X = stockdata[['s' + str(i) for i in range(1, 7)] + ['normed_tau']]
Y = stockdata['t0_bin']
X_train = X[stockdata.test > test_fraction]
X_test = X[stockdata.test < test_fraction]
Y_train = Y[stockdata.test > test_fraction]
Y_test = Y[stockdata.test < test_fraction]

classifier = MLPClassifier(
    hidden_layer_sizes=(len(timearray) - 1, len(timearray) + 1),
    activation='relu',
    solver='adam',
    tol=1.0e-6,
    alpha=0.005,
    verbose=True
)

print('Fitting the neural network...')

nntime_fitter = PMMLPipeline([('claasifier', classifier)])
nntime_fitter.fit(X_train, Y_train)

test_score = nntime_fitter.score(X_test, Y_test)
train_score = nntime_fitter.score(X_train, Y_train)
print('Test: {}'.format(test_score))
print('Train: {}'.format(train_score))
print('Fitting done.')

print('Writing output...')

pmml_name = 'SVDTimeNet.pmml'
xml_name = pmml_name.replace('pmml', 'xml')
sklearn2pmml(nntime_fitter, pmml_name, with_repr=True)

parser = ET.XMLParser(remove_blank_text=True)
net = ET.parse(pmml_name, parser)
root = net.getroot()
# namespace hassle
namespace = root.nsmap[None]
nsprefix = '{' + namespace + '}'
procinfo = root.find(nsprefix + 'MiningBuildTask')

# Save some metadata
name = ET.SubElement(procinfo, nsprefix + 'Title')
name.text = 'Neural network for time shift estimation'

# Information on use of the classifier
target = ET.SubElement(procinfo, nsprefix + 'IntendedUse')
basf2proc = ET.SubElement(target, nsprefix + 'basf2process')
basf2simulation = ET.SubElement(basf2proc, nsprefix + 'Simulation')
basf2simulation.text = 'yes'
basf2reconstruction = ET.SubElement(basf2proc, nsprefix + 'Reconstruction')
basf2reconstruction.text = 'yes'
sensorType = ET.SubElement(target, nsprefix + 'SensorType')
sensorType.text = 'all'
sensorSide = ET.SubElement(target, nsprefix + 'SensorSide')
sensorSide.text = 'all'

# information on training
training = ET.SubElement(procinfo, nsprefix + 'Training')
source = ET.SubElement(training, nsprefix + 'SampleSource')
source.text = 'Toy simulation'
genfunc = ET.SubElement(training, nsprefix + 'Waveform')
genfunc.text = 'beta-prime'
num_samples = ET.SubElement(training, nsprefix + 'SampleSize')
train_samples = ET.SubElement(num_samples, nsprefix + 'Training', {'n': str(int((1 - test_fraction) * args.n_samples))})
test_samples = ET.SubElement(num_samples, nsprefix + 'Test', {'n': str(int(test_fraction * args.n_samples))})
bounds.apply(
    lambda row: ET.SubElement(training, nsprefix + 'Parameter', **{u: str(v) for u, v in row.items()}), axis=1
)

netparams = ET.SubElement(procinfo, nsprefix + 'NetworkParameters')
inputLayer = ET.SubElement(netparams, nsprefix + 'NetworkLayer')
inputLayer.attrib['number'] = str(0)
inputLayer.attrib['kind'] = 'input'
inputLayer.attrib['size'] = str(7)  # 7 as in 6 APV samples + tau
n_hidden = len(classifier.hidden_layer_sizes)
for (iLayer, sz) in zip(range(1, 1 + n_hidden), classifier.hidden_layer_sizes):
    layer = ET.SubElement(netparams, nsprefix + 'NetworkLayer')
    layer.attrib['number'] = str(iLayer)
    layer.attrib['kind'] = 'hidden'
    layer.attrib['size'] = str(sz)
outputLayer = ET.SubElement(netparams, nsprefix + 'NetworkLayer')
outputLayer.attrib['number'] = str(n_hidden + 1)
outputLayer.attrib['kind'] = 'output'
outputLayer.attrib['size'] = str(len(timearray))

for field in root.find(nsprefix + 'DataDictionary'):
    if field.attrib['name'] == 't0_bin':
        for child in field:
            i = int(child.attrib['value'])
            child.attrib['lower'] = '{0:.3f}'.format(bins.loc[i, 'lower'])
            child.attrib['upper'] = '{0:.3f}'.format(bins.loc[i, 'upper'])
            child.attrib['midpoint'] = '{0:.3f}'.format(bins.loc[i, 'midpoint'])

net.write(xml_name, xml_declaration=True, pretty_print=True, encoding='utf-8')

print('Output saved.')

print('Saving fits...')

# #### Set up tau en/decoder
amp_index = bounds[bounds.value == 'amplitude'].index[0]
amp_range = (bounds.ix[amp_index, 'low'], bounds.ix[amp_index, 'high'])
tau_index = bounds[bounds.value == 'tau'].index[0]
tau_range = (bounds.ix[tau_index, 'low'], bounds.ix[tau_index, 'high'])
coder = tau_encoder(amp_range, tau_range)

# #### True values

Trues_test = stockdata[stockdata.test < test_fraction][['t0', 'amplitude', 'tau', 't0_bin', 'abin']]

# #### Predicted probabilities.

probs = nntime_fitter.predict_proba(X_test)

# ### Calculate time shifts and amplitudes from probabilities


def fitFromProb(fw, signals, p, tau, timearray):
    t_fit = np.average(timearray, weights=p)
    t_sigma = np.sqrt(np.average((timearray - t_fit)**2, weights=p))
    weights = fw(-t_fit + np.linspace(-dt, 4 * dt, 6, endpoint=True), tau=tau)
    weights[signals.values == 0.0] = 0.0
    norm = 1.0 / np.inner(weights, weights)
    a_fit = np.inner(signals, weights) * norm
    a_sigma = np.sqrt(norm)
    residuals = signals - a_fit * weights
    ndf = np.sum(np.ones_like(signals[signals > 0])) - 2  # Can't be less than 1
    chi2_ndf = np.inner(residuals, residuals) / ndf
    return pd.Series({
        't_fit': t_fit,
        't_sigma': t_sigma,
        'a_fit': a_fit,
        'a_sigma': a_sigma,
        'chi2_ndf': chi2_ndf
    })


probdf = pd.DataFrame(probs)
probdf.index = X_test.index
probdf.to_pickle('SVDTime_TrainingProbs_{0}.pkl'.format(args.n_samples))

fits = X_test.apply(
    lambda row: fitFromProb(
        betaprime_wave,
        row[['s' + str(i) for i in range(1, 7)]],
        probdf.ix[row.name],
        coder.decode(row['normed_tau']),
        timearray),
    axis=1
)
fits['t_true'] = Trues_test['t0']
fits['tau'] = Trues_test['tau']
fits['a_true'] = Trues_test['amplitude']
fits['t_bin'] = Trues_test['t0_bin']
fits['a_bin'] = Trues_test['abin']

fits.to_pickle('SVDTime_TrainingFits_{0}.pkl'.format(args.n_samples))

print('Writing classifier...')

with open('classifier.pkl', 'wb') as f:
    pickle.dump(classifier, f)

with open('classifier.txt', 'w') as cdump:
    cdump.write("Classifier coefficients:\n")
    for iLayer in range(len(classifier.coefs_)):
        cdump.write('Layer: {0}\n'.format(iLayer))
        nrows = classifier.coefs_[iLayer].shape[0]
        ncols = classifier.coefs_[iLayer].shape[1]
        cdump.write('Weights:\n')
        for col in range(ncols):
            s = " ".join([str(classifier.coefs_[iLayer][row, col]) for row in range(nrows)])
            s += "\n"
            cdump.write(s)
        # intercepts should have nrows dimension
        cdump.write('Intercepts:\n')
        s = " ".join([str(classifier.intercepts_[iLayer][col]) for col in range(ncols)])
        s += "\n"
        cdump.write(s)

print("Learning phase completed.")
