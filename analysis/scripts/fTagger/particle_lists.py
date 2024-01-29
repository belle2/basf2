# this script fills particle lists

# imports
from vertex import kFit
import modularAnalysis as ma
from categories import get_available_categories
from utils import get_Belle_or_Belle2


def fill_particle_lists(maskName='all', categories=[], path=None):
    """
    Fills the particle Lists for all categories.
    """

    # define the basic cut on each track
    trackCut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5 and p >= 0'

    readyParticleLists = []  # keep track of filled particle lists
    exp_type = get_Belle_or_Belle2()

    for category in categories:
        particleList = get_available_categories()[category].particleList

        if particleList in readyParticleLists:
            continue

        # Select particles in ROE for different categories according to mass hypothesis.
        if particleList == 'Lambda0:inRoe':
            if 'pi+:inRoe' not in readyParticleLists:
                ma.fillParticleList('pi+:inRoe', trackCut, path=path)
                readyParticleLists.append('pi+:inRoe')

            ma.fillParticleList('p+:inRoe', trackCut, path=path)
            ma.reconstructDecay(particleList + ' -> pi-:inRoe p+:inRoe', '1.00<=M<=1.23', False, path=path)
            kFit(particleList, 0.01, path=path)
            ma.matchMCTruth(particleList, path=path)
            readyParticleLists.append(particleList)

        else:
            # Filling particle list for actual category
            ma.fillParticleList(particleList, trackCut, path=path)
            readyParticleLists.append(particleList)

    # Additional particleLists for K_S0
    if exp_type == 'Belle':
        ma.cutAndCopyList('K_S0:inRoe', 'K_S0:mdst', 'extraInfo(ksnbStandard) == 1 and isInRestOfEvent == 1', path=path)
    else:
        if 'pi+:inRoe' not in readyParticleLists:
            ma.fillParticleList('pi+:inRoe', trackCut, path=path)
        ma.reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', False, path=path)
        kFit('K_S0:inRoe', 0.01, path=path)

    # Apply BDT-based LID
    default_list_for_lid_BDT = ['e+:inRoe', 'mu+:inRoe'] if exp_type == 'Belle2' else []
    list_for_lid_BDT = list(set(default_list_for_lid_BDT) & set(readyParticleLists))

    if list_for_lid_BDT:  # empty check
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=0,  # binary
                              binaryHypoPDGCodes=(11, 211))  # e vs pi
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=0,  # binary
                              binaryHypoPDGCodes=(13, 211))  # mu vs pi
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=1)  # Multiclass
