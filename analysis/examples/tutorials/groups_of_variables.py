event_variables = [
    'evtNum',
    'expNum',
    'productionIdentifier',
    'runNum']

# Variables for combined 2-lepton particle
kinematic_variables = ['px',
                       'py',
                       'pz',
                       'pt',
                       'p',
                       'E',
                       'M',
                       'ErrM',
                       'SigM',
                       'InvM',
                       'useCMSFrame(px)',
                       'useCMSFrame(py)',
                       'useCMSFrame(pz)',
                       'useCMSFrame(pt)',
                       'useCMSFrame(p)',
                       'useCMSFrame(E)']

cluster_variables = [
    'clusterE',
    'clusterReg']

track_variables = [
    'dr',
    'dx',
    'dy',
    'dz',
    'd0',
    'z0',
    'nCDCHits',
    'nPXDHits',
    'nSVDHits',
    'nVXDHits',
    'pValue']

mc_variables = [
    'genMotherID',
    'genMotherP',
    'genMotherPDG',
    'genParticleID',
    'isCloneTrack',
    'mcDX',
    'mcDY',
    'mcDZ',
    'mcDecayTime',
    'mcE',
    'mcErrors',
    'mcInitial',
    'mcP',
    'mcPDG',
    'mcPT',
    'mcPX',
    'mcPY',
    'mcPZ',
    'mcPhi',
    'mcVirtual',
    'nMCMatches']

pid_variables = [
    'kaonID',
    'pionID',
    'protonID',
    'muonID',
    'electronID']

roe_multiplicities = [
    'nROEKLMClusters']

recoil_kinematics = [
    'pRecoil',
    'pRecoilPhi',
    'pRecoilTheta',
    'pxRecoil',
    'pyRecoil',
    'pzRecoil']

# see BII-3874
flight_info = []

# see BII-3874
mc_flight_info = []

# see BII-3876
vertex = [
    'x',
    'x_uncertainty',
    'y',
    'y_uncertainty',
    'z',
    'z_uncertainty',
    'pValue']

# see BII-3876
mc_vertex = [
    'matchedMC(x)',
    'matchedMC(x_uncertainty)',
    'matchedMC(y)',
    'matchedMC(y_uncertainty)',
    'matchedMC(z)',
    'matchedMC(z_uncertainty)',
    'matchedMC(pValue)']

tag_vertex = [
    'TagVLBoost',
    'TagVLBoostErr',
    'TagVOBoost',
    'TagVOBoostErr',
    'TagVpVal',
    'TagVx',
    'TagVxErr',
    'TagVy',
    'TagVyErr',
    'TagVz',
    'TagVzErr',
]

mc_tag_vertex = [
    'MCDeltaT',
    'MCTagBFlavor',
    'TagVmcLBoost',
    'TagVmcOBoost',
    'mcLBoost',
    'mcOBoost',
    'mcTagVx',
    'mcTagVy',
    'mcTagVz',
]

momentum_uncertainty = [
    'E_uncertainty',
    'pxErr',
    'pyErr',
    'pzErr']

# see BII-3877
flavor_tagger = [
    'qrOutput(FBDT)'
]


def convert_to_daughter_vars(variables, daughter_number):
    daughter_variables = []
    for v in variables:
        daughter_variables.append('daughter(' + str(daughter_number) + ',' + v + ')')
    return daughter_variables


def convert_to_gd_vars(variables, daughter_number, granddaughter_number):
    gd_variables = []
    for v in variables:
        gd_variables.append('daughter(' + str(daughter_number) + ',daughter(' + str(granddaughter_number) + ',' + v + '))')
    return gd_variables


def make_mc(variables):
    mced_variables = []
    for v in variables:
        mced_variables.append('matchedMC(' + v + ')')
    return mced_variables
