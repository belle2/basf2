#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Thomas Keck 2017

"""
This file contains code to ensure backward compatibility for old FEI trainings with later releases.
In principle a FEI training is only supported for the release it was trained with.
However, to give the user some flexibility we can provide some code to adapt the FEI trainings to changes in the framework.
"""


def pid_renaming_oktober_2017():
    """ Adds aliases to map the old pid names to the newly introduced names. """
    # Backward compatibility (Remove in March 2018)
    from variables import variables as v
    v.addAlias('Kid_belle', 'kIDBelle')

    v.addAlias('eid', 'electronID')
    v.addAlias('muid', 'muonID')
    v.addAlias('piid', 'pionID')
    v.addAlias('Kid', 'kaonID')
    v.addAlias('prid', 'protonID')

    v.addAlias('eid_TOP', 'pidPairProbabilityExpert(11, 211, TOP)')
    v.addAlias('eid_ECL', 'pidPairProbabilityExpert(11, 211, ECL)')
    v.addAlias('eid_ARICH', 'pidPairProbabilityExpert(11, 211, ARICH)')
    v.addAlias('eid_dEdx', 'pidPairProbabilityExpert(11, 211, CDC, SVD)')

    v.addAlias('muid_TOP', 'pidPairProbabilityExpert(13, 211, TOP)')
    v.addAlias('muid_ECL', 'pidPairProbabilityExpert(13, 211, ECL)')
    v.addAlias('muid_ARICH', 'pidPairProbabilityExpert(13, 211, ARICH)')
    v.addAlias('muid_dEdx', 'pidPairProbabilityExpert(13, 211, CDC, SVD)')

    v.addAlias('Kid_TOP', 'pidPairProbabilityExpert(321, 211, TOP)')
    v.addAlias('Kid_ECL', 'pidPairProbabilityExpert(321, 211, ECL)')
    v.addAlias('Kid_ARICH', 'pidPairProbabilityExpert(321, 211, ARICH)')
    v.addAlias('Kid_dEdx', 'pidPairProbabilityExpert(321, 211, CDC, SVD)')

    v.addAlias('prid_TOP', 'pidPairProbabilityExpert(2212, 211, TOP)')
    v.addAlias('prid_ECL', 'pidPairProbabilityExpert(2212, 211, ECL)')
    v.addAlias('prid_ARICH', 'pidPairProbabilityExpert(2212, 211, ARICH)')
    v.addAlias('prid_dEdx', 'pidPairProbabilityExpert(2212, 211, CDC, SVD)')
