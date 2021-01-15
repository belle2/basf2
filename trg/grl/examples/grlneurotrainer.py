#!/user/bin/env python

import basf2
from ROOT import Belle2
import os
import glob

"""
Example script showing how to train neural networks
to be used with the CDCTriggerNeuroModule.

This script uses realistic values for the amount of training data
and the number of runs, so it will run a long time.
"""

# ------------ #
# user options #
# ------------ #

# set random seed
basf2.set_random_seed(1)

# paths for the trained networks, the training data and the log files
mlpdir = Belle2.FileSystem.findFile('trg/grl/data')
traindir = Belle2.FileSystem.findFile('trg/grl/data')
logdir = Belle2.FileSystem.findFile('trg/grl/data')
# filenames for the trained networks, the training data and the log files
mlpname = 'GRLNeuro.root'
trainname = 'GRLNeuroTraindata.root'
logname = 'GRLNeuroLog'  # file extensions are appended automatically

# number of threads to be used for parallel training
nthreads = 1


# ------------------------- #
# create path up to trigger #
# ------------------------- #

main = basf2.create_path()

main.add_module('Progress')
main.add_module('RootInput')

# ---------------- #
# add the training #
# ---------------- #

main.add_module('GRLNeuroTrainer',
                # output
                filename=os.path.join(mlpdir, mlpname),
                trainFilename=os.path.join(traindir, trainname),

                # network structure
                # nMLP=20,  # total number of sectors
                # multiplyHidden=False,  # set the number of hidden nodes directly
                # nHidden=[[10]],  # 1 hidden layer with 81 nodes for all sectors
                # i_cdc_sector=[0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,3],
                # i_ecl_sector=[0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4],
                nMLP=8,  # total number of sectors
                multiplyHidden=False,  # set the number of hidden nodes directly
                nHidden=[[100]],  # 1 hidden layer with 81 nodes for all sectors
                n_cdc_sector=1,
                n_ecl_sector=8,
                i_cdc_sector=[0*2+36*3, 0*2+36*3, 0*2+36*3, 0*2+36*3, 0*2+36*3, 0*2+36*3, 0*2+36*3, 0*2+36*3],
                i_ecl_sector=[0*3,     1*3,     2*3,     3*3,     4*3,     5*3,     6*3, 7*3],

                wMax=63.,  # limit weights to [-63, 63]
                # training parameters
                # multiplyNTrain=True,  # set training data relative to degrees of freedom
                # nTrainMax=10.,        # training data (10x degrees of freedom)
                # nTrainMin=10.,        # don't train if there is less than 10x DoF training data
                multiplyNTrain=False,  # set training data relative to degrees of freedom
                nTrainMax=2000,       # training data (10x degrees of freedom)
                nTrainMin=2000,       # don't train if there is less than 10x DoF training data
                nValid=1000,          # number of validation samples (to avoid overtraining)
                nTest=1000,           # number of test samples (to select best of several runs)
                # repeatTrain=10,      # train each sector 10x with different initial weights
                repeatTrain=1,        # train each sector 10x with different initial weights
                checkInterval=500,    # stop training if validation error does not improve for 500 epochs
                # maxEpochs=10000,      # stop training after 10000 epochs
                maxEpochs=1000,       # stop training after 10000 epochs
                nThreads=nthreads,    # number of parallel threads
                # log level
                logLevel=basf2.LogLevel.DEBUG,  # show some debug output
                debugLevel=50)

# show only the message of the debug output
# basf2.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)


# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
