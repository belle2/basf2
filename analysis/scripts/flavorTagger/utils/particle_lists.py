# this script fills particle lists

# imports
from .categories import get_available_categories
from .helpers import get_Belle_or_Belle2


def fill_particle_lists(maskName='all', categories=[], path=None):
    """
    Fills the particle Lists for all categories.
    """
    # imports
    from vertex import kFit
    import modularAnalysis as ma

    # define the basic cut on each track
    track_cut = 'isInRestOfEvent > 0.5 and passesROEMask(' + maskName + ') > 0.5 and p >= 0'

    ready_lists = []  # keep track of filled particle lists
    exp_type = get_Belle_or_Belle2()

    # fill category wise particle lists
    for category in categories:
        particle_list = get_available_categories()[category].particleList

        # check if duplicate
        if particle_list in ready_lists:
            continue

        # select particles in ROE for different categories according to mass hypothesis.
        if particle_list == 'Lambda0:inRoe':
            if 'pi+:inRoe' not in ready_lists:
                ma.fillParticleList('pi+:inRoe', track_cut, path=path)
                ready_lists.append('pi+:inRoe')

            ma.fillParticleList('p+:inRoe', track_cut, path=path)
            ma.reconstructDecay(particle_list + ' -> pi-:inRoe p+:inRoe', '1.00<=M<=1.23', False, path=path)
            kFit(particle_list, 0.01, path=path)
            ma.matchMCTruth(particle_list, path=path)
            ready_lists.append(particle_list)

        else:
            # Filling particle list for actual category
            ma.fillParticleList(particle_list, track_cut, path=path)
            ready_lists.append(particle_list)

    # Additional particleLists for K_S0
    if exp_type == 'Belle':
        ma.cutAndCopyList('K_S0:inRoe', 'K_S0:mdst', 'extraInfo(ksnbStandard) == 1 and isInRestOfEvent == 1', path=path)
    else:
        if 'pi+:inRoe' not in ready_lists:
            ma.fillParticleList('pi+:inRoe', track_cut, path=path)
        ma.reconstructDecay('K_S0:inRoe -> pi+:inRoe pi-:inRoe', '0.40<=M<=0.60', False, path=path)
        kFit('K_S0:inRoe', 0.01, path=path)

    # Apply BDT-based LID
    default_list_for_lid_BDT = ['e+:inRoe', 'mu+:inRoe'] if exp_type == 'Belle2' else []
    list_for_lid_BDT = list(set(default_list_for_lid_BDT) & set(ready_lists))

    if list_for_lid_BDT:  # empty check
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=0,  # binary
                              binaryHypoPDGCodes=(11, 211))  # e vs pi
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=0,  # binary
                              binaryHypoPDGCodes=(13, 211))  # mu vs pi
        ma.applyChargedPidMVA(particleLists=list_for_lid_BDT, path=path,
                              trainingMode=1)  # Multiclass
