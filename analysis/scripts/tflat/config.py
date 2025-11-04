#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

maskName = 'TFLATDefaultMask'
config = {  # ROE Mask
    'TFLAT_Mask': (
        maskName,
        'thetaInCDCAcceptance and p<infinity and p >= 0 and dr<1 and abs(dz)<3',
        'thetaInCDCAcceptance and clusterNHits>1.5 and [[E>0.08 and clusterReg==1] or [E>0.06 and clusterReg==2] or \
                    [E>0.06 and clusterReg==3]] and abs(clusterTiming) < 200'),
}

config.update({  # Input lists
    'trk_variable_list': [
        'charge',
        'useCMSFrame(p)',
        'useCMSFrame(cosTheta)',
        'useCMSFrame(phi)',
        'electronID',
        'muonID',
        'kaonID',
        'pionID',
        'protonID',
        'pidPairProbabilityExpert(321, 211, CDC)',
        'pidPairProbabilityExpert(321, 211, TOP)',
        'pidPairProbabilityExpert(321, 211, ARICH)',
        'pidPairProbabilityExpert(11, 211, TOP)',
        'pidPairProbabilityExpert(11, 211, ARICH)',
        'pidPairProbabilityExpert(11, 211, ECL)',
        'pidPairProbabilityExpert(13, 211, TOP)',
        'pidPairProbabilityExpert(13, 211, ARICH)',
        'pidPairProbabilityExpert(13, 211, KLM)',
        'pidPairProbabilityExpert(211, 321, TOP)',
        'pidPairProbabilityExpert(211, 321, ARICH)',
        # 'nCDCHits/56',
        'nPXDHits/2',
        'nSVDHits/8',
        # 'dxdiff',
        # 'dydiff',
        # 'dzdiff',
        'dz',
        'dr',
        # 'chiProb',
        'clusterEoP',
        'clusterLAT',
    ],

    'ecl_variable_list': [
        'useCMSFrame(p)',
        'useCMSFrame(cosTheta)',
        'useCMSFrame(phi)',
        'clusterE1E9',
        'clusterE9E21',
        'clusterLAT',
    ],

    'roe_variable_list': [
        'countInList(gamma:tflat)/8',
        'countInList(pi+:tflat)/6',
        'NumberOfKShortsInRoe',
        # 'isPXDOn',
        'ptTracksRoe('+maskName+')',
    ],
})

config.update({  # Architecture parameters
    'parameters': {
        'seed': None,
        "clip_value": 4,
        'mask_value': 0,
        'num_trk': 10,
        'num_trk_features': len(config['trk_variable_list']),
        'num_ecl': 20,
        'num_ecl_features': len(config['ecl_variable_list']),
        'num_roe': 1,
        'num_roe_features': len(config['roe_variable_list']),
        "num_transformer_blocks": 6,
        "num_heads": 4,
        "embedding_dims": 128,
        "mlp_hidden_units_factors": [2, 1,],
        "dropout_rate": 0.1,
        }
})

config.update({  # Training parameters
    'weight_decay': 0.0001,
    'initial_learning_rate': 1e-4,
    'decay_steps': 2000000,
    'alpha': 1e-1,
    'patience': 10,
    'batch_size': 256,
    'epochs': 200,
    'train_valid_fraction': .92,
})
