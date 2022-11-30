##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import hashlib
import os
import subprocess

import basf2 as b2
import pdg


# Check the basf2 location
basf2_dir = os.environ.get['BELLE2_LOCAL_DIR', os.environ.get['BELLE2_RELEASE_DIR']]

# Generation parameters
mg_steeringtemplate = f'{basf2_dir}/generators/madgraph/examples/run_darkphoton.steeringtemplate'
mg_nevents = '1000'
mg_beamenergy = '10.58/2.'
b2_seed = b2.get_random_seed().encode('utf-8')
mg_seed = f'{int(hashlib.sha256(b2_seed).hexdigest(), 16) % 10**8}'

# Models parameters
mg_model = f'{basf2_dir}/generators/madgraph/models/Dark_photon_UFO'
mg_generate = 'e+ e- > a ap, ap > DM DM~'
mAp = 1.0
mg_parameter_mAp = str(mAp)
mg_parameter_mDM = str(mAp / 3.0)
mg_parameter_gDM = '0.1'
mg_parameter_kappa = '0.001'
# this sets the width to 1 keV: switch to 'auto' for allowing MadGraph to compute the width using mAp, mDM and gDM
mg_parameter_wAp = '0.000001'

# Path to output directory
mg_output = f'Dark_photon_mass_{int(mAp)}'
if not os.path.exists(mg_output):
    os.mkdir(mg_output)
else:
    import shutil  # nowa
    shutil.rmtree(mg_output, ignore_errors=True)
    os.mkdir(mg_output)

# Other stuffs
mg_externals = 'mg5_aMC'  # MadGraph executable (from the externals)
mg_steeringfile = f'{mg_output}/run_darkphoton.steering'  # MadGraph steering file (will be created on-the-fly later)
# MadGraph run_card to be used (param_card is generated automatically)
mg_runcard = f'{basf2_dir}/generators/madgraph/cards/run_card.dat'

# Write the MadGraph steering file
mydict = {
    'MGMODEL': mg_model,
    'MGGENERATE': mg_generate,
    'MGOUTPUT': mg_output,
    'MGRUNDCARD': mg_runcard,
    'MGBEAMENERGY': mg_beamenergy,
    'MGNEVENTS': mg_nevents,
    'MGSEED': mg_seed,
    'MGPARAMETER_mAp': mg_parameter_mAp,
    'MGPARAMETER_wAp': mg_parameter_wAp,
    'MGPARAMETER_mDM': mg_parameter_mDM,
    'MGPARAMETER_gDM': mg_parameter_gDM,
    'MGPARAMETER_kappa': mg_parameter_kappa,
}
with open(mg_steeringtemplate, 'r') as template:
    data = template.read()
    for (key, value) in mydict.items():
        data = data.replace(key, value)
steering = open(mg_steeringfile, 'w')
steering.write(data)
steering.close()

# Run MadGraph and "gunzip" output file
subprocess.check_call([mg_externals, mg_steeringfile])
subprocess.check_call(['gunzip', f'{mg_output}/Events/run_01/unweighted_events.lhe.gz'])

# Run basf2
pdg.add_particle('Ap', 4900023, mAp, 0.000001, 0, 2)
pdg.add_particle('DM', 4900101, mAp / 3.0, 0.000001, 0, 1)
pdg.add_particle('anti-DM', -4900101, mAp / 3.0, 0.000001, 0, 1)

main = b2.Path()

main.add_module('LHEInput',
                expNum=0,
                runNum=0,
                inputFileList=b2.find_file(f'{mg_output}/Events/run_01/unweighted_events.lhe'),
                makeMaster=True,
                useWeights=False,
                nInitialParticles=2,
                nVirtualParticles=1,
                wrongSignPz=True)

main.add_module('BoostMCParticles')

main.add_module('SmearPrimaryVertex')

main.add_module('Progress')

main.add_module('RootOutput',
                outputFileName=f'Dark_photon_mass_{int(mAp)}.root')

b2.process(main)
