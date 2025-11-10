#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
 This script can be used to train the FEI on a cluster like available at KEKCC
 All you need is a basf2 steering file (see analysis/examples/FEI/ ) and some MC O(100) million
 The script will automatically create some directories
  - collection containing weight files, monitoring files and other stuff
  - jobs containing temporary files during the training (can be deleted afterwards)

 The distributed script automatically spawns jobs on the cluster (or local machine),
 and runs the steering file on the provided MC.
 Since a FEI training requires multiple runs over the same MC, it does so multiple times.
 The output of a run is passed as input to the next run (so your script has to use RootInput and RootOutput).

 In between it calls the do_trainings function of the FEI, to train the multivariate classifiers of the FEI
 at each stage.

 At the end it produces summary outputs using printReporting.py and latexReporting.py
 (this will only work if you use the monitoring mode)
 In addition, a summary file for each mva training is produced using basf2_mva_evaluate.

 If your training fails for some reason (e.g. a job fails on the cluster),
 the FEI will stop, you can fix the problem and resume the training using the -x option.
 This requires some expert knowledge, because you have to know how to fix the occurred problem
 and at which step you have to resume the training.

 After the training the weight files will be stored in the localdb in the collection directory
 You have to upload these local database to the Belle II Condition Database if you want to use the FEI everywhere.
 Alternatively you can just copy the localdb to somewhere and use it directly.

 Example:
 python3 ~/release/analysis/scripts/fei/distributed.py
   -s kekcc2
   -f ~/release/analysis/examples/FEI/B_generic.py
   -w /home/belle2/tkeck/group/B2TauNuWorkspace_2/new_fei
   -n 100
   -d $(ls /ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000786/s00/e0000/4S/r00000/mixed/sub01/*.root
        | head -n 50)
      $(ls /ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000788/s00/e0000/4S/r00000/charged/sub01/*.root
        | head -n 50)
"""


import subprocess
import sys
import os
import argparse
import glob
import time
import stat
import shutil
import pickle
import json
import b2biiConversion
import fei
import basf2_mva


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    # FEI defines own command line options, therefore we disable
    # the ROOT command line options, which otherwise interfere sometimes.
    # Always avoid the top-level 'import ROOT'.
    import ROOT  # noqa
    ROOT.PyConfig.IgnoreCommandLineOptions = True
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--steeringFile', dest='steering', type=str, required=True,
                        help='Steering file. Calls fei.get_path()')
    parser.add_argument('-w', '--workingDirectory', dest='directory', type=str, required=True,
                        help='Working directory for basf2 jobs. On KEKCC, this must NOT be on HSM!')
    parser.add_argument('-l', '--largeDirectory', dest='large_dir', type=str, default='',
                        help='Directory to store large files')
    parser.add_argument('-n', '--nJobs', dest='nJobs', type=int, default=100,
                        help='Number of jobs')
    parser.add_argument('-d', '--data', dest='data', type=str, required=True, action='append', nargs='+',
                        help='Data files in bash expansion syntax or as process_url')
    parser.add_argument('-x', '--skip-to', dest='skip', type=str, default='',
                        help='Skip setup of directories')
    parser.add_argument('-o', '--once', dest='once', action='store_true',
                        help='Execute just once time, instead of waiting until a Summary is produced.')
    parser.add_argument('-s', '--site', dest='site', type=str, default='kekcc',
                        help='Site to use [kekcc|kitekp|local]')
    parser.add_argument('-e', '--end', dest='end', type=int, default=6,
                        help='Stage at which to end the training')
    parser.add_argument('-r', '--retrain', dest='retrain', type=int, default=-1,
                        help='Stage at which to retrain the training')
    parser.add_argument('-v', '--validation', dest='validation', type=float,
                        default=0.2, help='Fraction of data to use for validation')
    args = parser.parse_args()
    return args


def get_job_script():
    """
    Create a bash file which will be dispatched to the batch system.
    The file will run basf2 on the provided MC or the previous output
    using the provided steering file.
    """
    job_script = """
        if [ -f "basf2_input.root" ]; then
          INPUT="basf2_input.root"
        else
          INPUT="input_*.root"
        fi
        time basf2 -l error ../../collection/basf2_steering_file.py -i "$INPUT" \
        -o basf2_output.root &> my_output_hack.log || touch basf2_job_error
        touch basf2_finished_successfully
    """
    return job_script


def setup(args, fullOverwrite=True):
    """
    Setup all directories, create job_scripts, split up MC into chunks
    which are processed by each job. Create symlinks for databases.
    """
    print(f'FEI-distributed-setup: Setup environment in {args.directory} with overwrite {fullOverwrite}')
    currDir = os.getcwd()

    os.chdir(args.directory)
    # Search and partition data files into even chunks
    data_files = []

    for x in args.data:
        for y in x:
            if (y.startswith("http://") or y.startswith("https://")):
                data_files += b2biiConversion.parse_process_url(y)
            else:
                data_files += glob.glob(y)
    print(f'FEI-distributed-setup: Found {len(data_files)} MC files')
    file_sizes = []
    for file in data_files:
        file_sizes.append(os.stat(file).st_size)
    data_files_sorted = [x for _, x in sorted(zip(file_sizes, data_files))]
    n = int(len(data_files) / args.nJobs)
    if n < 1:
        raise RuntimeError(f'Too few MC files {len(data_files)} for the given number of jobs {args.nJobs}')
    data_chunks = [data_files_sorted[i::args.nJobs] for i in range(args.nJobs)]

    # Create needed directories
    print(f'FEI-distributed-setup: Create environment in {args.directory}')
    if fullOverwrite:
        shutil.rmtree('collection', ignore_errors=True)
        os.mkdir('collection')
        if os.path.isfile('collection/Summary.pickle'):
            os.remove('collection/Summary.pickle')
        os.mkdir('collection/localdb')
    shutil.rmtree('jobs', ignore_errors=True)
    os.mkdir('jobs')
    if args.large_dir:
        if not os.path.isdir(args.large_dir):
            raise RuntimeError('FEI-distributed-setup: Large dir does not exist. Please make sure it does.')

    shutil.copyfile(args.steering, 'collection/basf2_steering_file.py')

    for i in range(args.nJobs):
        # Create job directory
        os.mkdir(f'jobs/{i}')
        job_script = get_job_script()
        with open(f'jobs/{i}/basf2_script.sh', 'w') as f:
            f.write(job_script)
            os.chmod(f.fileno(), stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)
        # Symlink initial input data files
        for j, data_input in enumerate(data_chunks[i]):
            os.symlink(data_input, f'jobs/{i}/input_{j}.root')
        # Symlink weight directory and basf2_path
        os.symlink('../../collection/localdb', f'jobs/{i}/localdb')
    os.chdir(currDir)


def create_report(args):
    """
    Dumps Summary.pickle to JSON for easy inspection.
    Create all the reports for the FEI training and the individual mva trainings.
    This will only work if
      1) Monitoring mode is used (see FeiConfiguration)
      2) The system has enough memory to hold the training data for the mva classifiers
    If this fails you can also copy the collection directory somewhere and
    execute the commands by hand.
    """
    import ROOT  # noqa
    print('FEI-distributed-report: Create report')
    currDir = os.getcwd()

    os.chdir(f'{args.directory}/collection')
    with open('Summary.pickle', 'rb') as file:
        summary = pickle.load(file)

    summary_dict = {particle.identifier:
                    {'mvaConfig': particle.mvaConfig._asdict(),
                     'channels': [{field: (value._asdict() if field in ['mvaConfig', 'preCutConfig'] else value) for
                                   field, value in channel._asdict().items()} for channel in particle.channels],
                     'preCutConfig': particle.preCutConfig._asdict(),
                     'postCutConfig': particle.postCutConfig._asdict()}
                    for particle in summary[0]}
    summary_dict.update({'feiConfig': summary[1]._asdict()})

    with open('Summary.json', 'w') as summary_json_file:
        json.dump(summary_dict, summary_json_file, indent=4)
    print('FEI-distributed-report: read Summary.pickle and wrote Summary.json')

    ret = subprocess.call('basf2 fei/printReporting.py summary.txt', shell=True)
    if ret != 0:
        raise RuntimeError('Error during printReporting.py')
    else:
        print('FEI-distributed-report: Created summary.txt')
    ret = subprocess.call('basf2 fei/latexReporting.py summary.tex', shell=True)
    if ret != 0:
        raise RuntimeError('Error during latexReporting.py')
    else:
        print('FEI-distributed-report: Created summary.tex')

    print('FEI-distributed-report: Creating *.zip files for the mva evaluation.')
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    stage_particles = fei.core.get_stages_from_particles(particles)
    ret = 0
    for istage in range(len(stage_particles)):
        stage = stage_particles[istage]
        for particle in stage:
            for channel in particle.channels:
                if basf2_mva.available(f'{channel.label}.xml') and not fei.core.Teacher.check_if_weightfile_is_fake(
                        f'{channel.label}.xml'):
                    treeName = ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(f'{channel.label} variables')
                    try:
                        result = subprocess.run(
                            f"basf2_mva_evaluate.py -id '{channel.label}.xml' -train 'training_input.root' "
                            f"-data 'validation_input.root' "
                            f"--treename '{treeName}' "
                            f"-o '../{channel.label}.zip'",
                            shell=True,
                            capture_output=True,
                            text=True,
                            check=True)
                        print(f"FEi-distributed-report: Created {channel.label}.zip, success status: ")
                        print(result.stdout)
                        print(f"FEi-distributed-report: err-output evaluation of {channel.label}.xml")
                        print(result.stderr)
                    except subprocess.CalledProcessError as e:
                        print(f"FEi-distributed-report: Error during evaluation of {channel.label}.xml")
                        print(e.stderr)
                        ret = 1
                else:
                    print(f"FEi-distributed-report: Skipping evaluation of {channel.label}.xml as it is a fake weight file")
    os.chdir(currDir)
    print('FEI-distributed-report: DONE creating reports')
    return ret == 0


def remove_objects(file_path, objectNames):
    """
    Remove objects from a ROOT file
    """
    if os.path.islink(file_path):
        file_path = os.readlink(file_path)
    import ROOT  # noqa
    root_file = ROOT.TFile.Open(file_path, "UPDATE")
    key_names = [key.GetName() for key in root_file.GetListOfKeys()]

    print(f'All keys in {file_path}: {key_names}')
    for obj_name in key_names:
        if any([ROOT.Belle2.MakeROOTCompatible.makeROOTCompatible(objectName) == obj_name for objectName in objectNames]):
            print(f'Removed {obj_name}')
            root_file.Delete(f'{obj_name};*')
    root_file.Write("", ROOT.TObject.kOverwrite)
    root_file.Close()


def remove_helper(args, channelLabel, configuration):
    """
    Helper function to remove all files related to a channelLabel
    """
    curDir = os.getcwd()
    os.chdir(f'{args.directory}/collection')
    print(f'FEI-REtrain: Cleaning {channelLabel}')
    # 3.2/ remove .zip, trained BDTs, logs, etc. in collection:
    removeFiles = glob.glob(f'{channelLabel}*')
    removeFiles += glob.glob(f'{channelLabel.split(" ")[0]}*.png')
    removeFiles += glob.glob(f'../{channelLabel}*.zip')
    print(f'FEI-REtrain: Removing {channelLabel}* files in total {len(removeFiles)}')
    for f in removeFiles:
        os.remove(f)
    # 3.3/ clean localdb in collection: # not for now

    os.chdir(args.directory)
    # 3.4/ clean symlinks:
    # - remove .xml symlinks with particle identifier and Summary.pickle
    symlinks = glob.glob(f'jobs/*/{channelLabel}.xml')
    print(f'FEI-REtrain: Removing symlinks for {channelLabel} in total {len(symlinks)}')
    for f in symlinks:
        os.unlink(f)
    os.chdir(curDir)


def clean_higher_stages(args):
    """
    Cleans the higher stages of the training.
    This is needed if you want to retrain the BDTs
    """
    print('FEI-distributed-clean_higher_stages: Cleaning higher stages')
    currDir = os.getcwd()
    os.chdir(f'{args.directory}/collection')
    # 3.1/ basf2_input.root Are pruned before this, using a special run of jobs with roundMode == 3

    stages = fei.core.get_stages_from_particles(particles)
    channels_to_remove = []
    # list of all .xml weight files
    xmlfiles = glob.glob("*.xml")
    for i in range(args.retrain-1, len(stages)):
        print(f'FEI-distributed-clean_higher_stages: Cleaning stage {i}')
        for particle in stages[i]:
            for channel in particle.channels:
                if f'{channel.label}.xml' in xmlfiles:
                    xmlfiles.remove(f'{channel.label}.xml')  # this will force to retrain
                remove_helper(args, channel.label, configuration)
                # 3.5/ prune Monitoring training plots in collection:
                channels_to_remove.append(channel.label)
    print(f"FEI-REtrain: Channels to remove: {channels_to_remove}")

    for xf in xmlfiles:
        print(f'FEI-REtrain: {xf} will not be retrained!')

    # 3.6/ prune Monitoring plots in collection:
    partIds = []
    for i in range(args.retrain-1, len(stages)):
        for particle in stages[i]:
            partIds.append(particle.identifier)
            if "J/psi" in particle.identifier:
                partIds.append(particle.identifier.replace("J/psi", "Jpsi"))
    print(f'FEI-REtrain: particles to remove: {partIds}')

    root_files = glob.glob('*.root')
    for f in root_files:
        if any([x in f for x in ['Monitor_PostReconstruction_AfterMVA', 'Monitor_Pre', 'Monitor_TrainingData']]):
            print(f'FEI-REtrain: Removing branches in {f}')
            remove_objects(f, objectNames=channels_to_remove)
        elif 'Monitor_PostReco' in f:
            print(f'FEI-REtrain: Removing branches in {f}')
            remove_objects(f, objectNames=partIds)
        elif 'Monitor_Final' in f:
            print(f'FEI-REtrain: Removing branches in {f}')
            remove_objects(f, objectNames=[f'{partId} variables' for partId in partIds])
        elif any([x in f for x in ['training_input', 'validation_input']]):
            print(f'FEI-REtrain: Removing branches in {f}')
            remove_objects(f, objectNames=[f'{ch} variables' for ch in channels_to_remove])

    # 3.7/ clean job directories:
    # - remove all root files except mcParticlesCount.root and basf2_input.root
    # - particles in Particle list are removed in a special run of jobs using RemoveParticlesNotInList module
    os.chdir(args.directory)
    root_files = glob.glob('jobs/*/*.root')
    print('FEI-REtrain: Removing job root files (not mcParticlesCount.root, basf2_input.root or basf2_output.root)')
    for f in root_files:
        if 'mcParticlesCount.root' not in f and 'basf2_input.root' not in f and 'basf2_output.root' not in f:
            os.remove(f)
    os.chdir(currDir)
    print('FEI-distributed-clean_higher_stages: Done cleaning higher stages')


def clean_monitoring(args, ijob=None):
    """
    Cleans the monitoring files in the jobs directory and the training_input.root files
    """
    if ijob is None:
        files = glob.glob('jobs/*/Monitor_*.root')
        files += glob.glob('jobs/*/training_input.root')
    else:
        files = glob.glob(f'jobs/{ijob}/Monitor_*.root')
        files += glob.glob(f'jobs/{ijob}/training_input.root')

    for f in files:
        os.remove(f)


def clean_job_directory(args):
    """
    Cleans the job directory for the next iteration
    Meaning we remove all logs
    """
    files = glob.glob('jobs/*/basf2_finished_successfully')
    files += glob.glob('jobs/*/error.log')
    files += glob.glob('jobs/*/output.log')
    files += glob.glob('jobs/*/basf2_job_error')
    for f in files:
        os.remove(f)
    for i in range(args.nJobs):
        nHackLogs = len(glob.glob(f'jobs/{i}/my_output_hack.log.backup_*'))
        if os.path.isfile(f'jobs/{i}/my_output_hack.log'):
            os.rename(f'jobs/{i}/my_output_hack.log', f'jobs/{i}/my_output_hack.log.backup_{nHackLogs}')


def submit_job(args, i):
    """
    Submits a job to the desired batch system.
    Currently we can run on KEKCC (long queue), KEKCC (dedicated FEI queue),
    EKP @ KIT, or your local machine
    """
    # Synchronize summaries
    currDir = os.getcwd()
    os.chdir(args.directory)
    if os.path.isfile('collection/Summary.pickle'):
        shutil.copyfile('collection/Summary.pickle', f'jobs/{i}/Summary.pickle')

    os.chdir(f'{args.directory}/jobs/{i}/')
    if args.site == 'kekcc':
        ret = subprocess.call("bsub -q l -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kekccs':
        ret = subprocess.call("bsub -q s -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif 'kekccX' in args.site:
        ret = subprocess.call(f"bsub -q l -n {args.site[6:]} -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kekcc2':
        ret = subprocess.call("bsub -q b2_fei -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kitekp':
        ret = subprocess.call("qsub -cwd -q express,short,medium,long -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'local':
        subprocess.Popen(['bash', './basf2_script.sh'])
        ret = 0
    else:
        raise RuntimeError(f'Given site {args.site} is not supported')
    os.chdir(currDir)
    return ret == 0


def missingSymlinks(args, printing=False):
    """
    Check if all xml files are present in the job directories.
    If not, create a symlink to the collection directory
    """
    currDir = os.getcwd()
    os.chdir(f'{args.directory}/collection')
    xmlfiles = glob.glob("*.xml")
    for i in range(args.nJobs):
        for xmlfile in xmlfiles:
            if not os.path.isfile(f'../jobs/{i}/{xmlfile}'):
                if printing:
                    print("FEI-missing-symlinks: Added missing symlink to ", xmlfile, " in job directory ", i)
                os.symlink(f'../../collection/{xmlfile}', f'../jobs/{i}/{xmlfile}')
    os.chdir(currDir)


def do_trainings(args):
    """
    Trains the multivariate classifiers for all available training data in
    the collection directory, which wasn't trained yet.
    This is called once per iteration
    """
    currDir = os.getcwd()
    os.chdir(f'{args.directory}/collection')
    if not os.path.isfile('Summary.pickle'):
        return
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    weightfiles = fei.do_trainings(particles, configuration)
    print('FEI-distributed-do_trainings: Finished trainings!!')

    os.chdir(args.directory)
    for i in range(args.nJobs):
        for weightfile_on_disk, _ in weightfiles:
            if not os.path.isfile(f'jobs/{i}/{weightfile_on_disk}'):
                os.symlink(f'../../collection/{weightfile_on_disk}', f'jobs/{i}/{weightfile_on_disk}')

    # Check if some xml files are missing
    missingSymlinks(args)
    if configuration.cache >= args.end:
        print('FEI-distributed-do_trainings: Finished last training')
        update_pickle(args, configuration.cache, 2)
    os.chdir(currDir)


def jobs_finished(args):
    """
    Check if all jobs already finished.
    Throws a runtime error of it detects an error in one of the jobs
    """
    currDir = os.getcwd()
    os.chdir(args.directory)
    finished = glob.glob('jobs/*/basf2_finished_successfully')
    failed = glob.glob('jobs/*/basf2_job_error')
    if len(failed) > 0:
        raise RuntimeError(f'basf2 execution failed! Error occurred in: {str(failed)}')
    os.chdir(currDir)
    return len(finished) == args.nJobs


def merge_root_files(args, stage, roundMode=0):
    """
    Merges all produced ROOT files of all jobs together
    and puts the merged ROOT files into the collection directory.
    This affects mostly
      - the training data for the multivariate classifiers
      - the monitoring files
    """
    print('FEI-distributed-merge_root_files: Merging root files in stage', stage)
    if roundMode == 3:
        print(f'FEI-distributed-merge_root_files --> FEI-REtrain: Cleaning up training data after stage {args.retrain}')
        clean_higher_stages(args)
        return

    currDir = os.getcwd()
    os.chdir(args.directory)

    rootfiles = []
    for f in glob.glob('jobs/0/*.root'):
        f = os.path.basename(f)
        if f in ['basf2_input.root', 'basf2_output.root', 'validation_input.root']:
            continue
        if f.startswith('input_'):
            continue
        if os.path.isfile(f'collection/{f}') and f in ['mcParticlesCount.root', 'Monitor_FSPLoader.root']:
            # done only once, TODO potential problem if you add additional particle PDG code on retraining?
            continue
        if (roundMode == 1) and (('PostReconstruction' in f) or (f == 'Monitor_Final.root')):
            # ignore when only preReco
            continue
        if (roundMode == 2) and (('PreReconstruction' in f) or (f in ['Monitor_TrainingData.root', 'training_input.root'])):
            # ignore when only postReco
            continue
        rootfiles.append(f)
    if len(rootfiles) == 0:
        print('There are no root files to merge in jobs/0/*.root')
    else:
        print('Merge the following files', rootfiles)
        os.chdir(f'{args.directory}/collection')
        for f in rootfiles:
            inputs = [f'../jobs/{i}/{f}' for i in range(args.nJobs)]
            cmd = f"analysis-fei-mergefiles -o {f} -i " + " ".join(inputs)
            if 'training_input' in f:
                cmd += f" -s {1.0-args.validation}"
            ret = subprocess.call(cmd, shell=True)
            if ret != 0:
                raise RuntimeError('Error during merging root files')

            # Replace mcParticlesCount.root with merged file in all directories
            # so that the individual jobs calculate the correct mcCounts and sampling rates
            if f == 'mcParticlesCount.root':
                for i in inputs:
                    os.remove(i)
                    os.symlink('../../collection/mcParticlesCount.root', i)
        os.chdir(currDir)
    print('FEI-distributed-merge_root_files: Done merging root files in stage', stage)


def update_input_files(args):
    """
    Updates the input files.
    For the first iteration the input files are the MC provided by the user.
    After each training this function replaces the input with the output of the previous iteration.
    Effectively this caches the whole DataStore of basf2 between the iterations.
    """
    currDir = os.getcwd()
    os.chdir(args.directory)
    print(f'FEI-distributed-update_input_files: Update input files, {os.getcwd()}')
    for i in range(args.nJobs):
        output_file = f'jobs/{i}/basf2_output.root'
        input_file = f'jobs/{i}/basf2_input.root'
        if args.large_dir:
            real_input_file = f'{args.large_dir}/basf2_input_{i}.root'
            shutil.move(output_file, real_input_file)
            if os.path.isfile(input_file):
                os.remove(input_file)
            os.symlink(real_input_file, input_file)
        else:
            shutil.move(output_file, input_file)
    # Saves the Summary.pickle of the first job to the collection directory
    # so we can keep track at which stage of the reconstruction the FEI is currently.
    cache, roundMode = get_training_cache(args)
    if roundMode != 2:
        shutil.copyfile('jobs/0/Summary.pickle', 'collection/Summary.pickle')

    cache, roundMode = get_training_cache(args)
    if roundMode == 1:
        roundMode = 0
    if roundMode == 3:
        roundMode = 1
    update_pickle(args, roundMode=roundMode)
    print('FEI-distributed-update_input_files: Updated input files to stage ',
          cache, 'with roundMode', roundMode, '. DONT do this again!')
    os.chdir(currDir)


def update_pickle(args, cache=None, roundMode=None):
    """
    Updates the pickle file in the collection directory.
    This is needed to keep track of the current stage of the training
    and the roundMode.
    """
    currDir = os.getcwd()
    os.chdir(args.directory)
    particles, configuration = pickle.load(open('collection/Summary.pickle', 'rb'))
    os.remove('collection/Summary.pickle')
    if cache is None:
        cache = configuration.cache
    if roundMode is None:
        roundMode = configuration.roundMode
    os.chdir(f'{args.directory}/collection')
    fei.save_summary(particles, configuration, cache, roundMode)
    os.chdir(currDir)
    return particles, configuration


def get_training_cache(args):
    """
    Checks if the FEI training is still ongoing.
    The training is finished if the FEI reached stage 7
    """
    if not os.path.isfile(f'{args.directory}/collection/Summary.pickle'):
        return -1, 0
    particles, configuration = pickle.load(open(f'{args.directory}/collection/Summary.pickle', 'rb'))
    # return configuration.cache != 7
    return configuration.cache, configuration.roundMode


# ================================================================
if __name__ == '__main__':
    pid = os.getpid()
    print(f'FEI training nohup job ID: {pid}', flush=True)

    args = getCommandLineOptions()
    os.chdir(args.directory)

    # If the user wants resume an existing training
    # we check at which step he wants to resume and
    # try to perform the necessary steps to do so
    if args.skip:
        print('FEI-distributed: Skipping setup')
        start = 0
        if args.skip == 'clean':
            start = 1
        elif args.skip == 'update':
            start = 2
        elif args.skip == 'merge':
            start = 3
        elif args.skip == 'wait':
            start = 4
        elif args.skip == 'submit':
            start = 5
        elif args.skip == 'resubmit':
            start = 6
        elif args.skip == 'report':
            start = 7
        elif args.skip == 'run':
            start = 0
        elif args.skip == 'rebase':
            start = -1
        elif args.skip == 'retrain':
            start = -2
        else:
            raise RuntimeError(f'Unknown skip parameter {args.skip}')

        if start == 7:
            print('FEI-distributed: (report) Create full report')
            create_report(args)
            sys.exit(0)

        # The user wants to submit the jobs again
        if start >= 5:
            print('Submitting jobs')
            # if not resubmit also retrain BDTs if needed
            if start == 5:
                if get_training_cache(args)[0] is None or get_training_cache(
                        args)[0] <= 0 or get_training_cache(args)[1] == 3 or get_training_cache(args)[1] == 1:
                    print('FEI-distributed: no training, because cache is None or <= 0, or roundMode is 3 or 1')
                else:
                    print('FEI-distributed: (train) Do available trainings for stage: ', get_training_cache(args))
                    do_trainings(args)

            print('FEI-distributed: (submit) Submitting jobs in mode: ', get_training_cache(args))
            for i in range(args.nJobs):
                # The user wants to resubmit jobs, this means the training of some jobs failed
                # We check which jobs contained an error flag, and were not successful
                # These jobs are submitted again, other jobs are skipped (continue)
                error_file = f'jobs/{i}/basf2_job_error'
                success_file = f'jobs/{i}/basf2_finished_successfully'
                output_log_file = f'jobs/{i}/output.log'
                error_log_file = f'jobs/{i}/error.log'

                if start >= 6:
                    if (not os.path.isfile(error_file) and os.path.isfile(success_file)):
                        continue
                    else:
                        print(f'FEI-distributed: (resubmit) Resubmitting job {i}')
                clean_monitoring(args, i)

                if os.path.isfile(error_file):
                    os.remove(error_file)
                if os.path.isfile(success_file):
                    os.remove(success_file)
                if os.path.isfile(output_log_file):
                    os.remove(output_log_file)
                if os.path.isfile(error_log_file):
                    os.remove(error_log_file)
                # Reset Summary file
                shutil.copyfile('collection/Summary.pickle', f'jobs/{i}/Summary.pickle')
                if not submit_job(args, i):
                    raise RuntimeError('Error during submitting job')

        if start >= 4:
            print('FEI-distributed: (wait) Wait for jobs to end')
            while not jobs_finished(args):
                time.sleep(40)

        if start >= 3:
            print('FEI-distributed: (merge) Merge ROOT files with training_input.root: ', get_training_cache(args))
            merge_root_files(args, get_training_cache(args)[0], roundMode=get_training_cache(args)[1])

        if start >= 2:
            print('FEI-distributed: (update) Update input files with summary: ', get_training_cache(args)[1])
            update_input_files(args)

        if start >= 1:
            print('FEI-distributed: (clean) Clean job directory')
            clean_job_directory(args)

        if start == 0:
            print('FEI-distributed: (run) Resuming training')
            tmp_cache, tmp_roundMode = get_training_cache(args)
            if tmp_roundMode == 2:
                update_pickle(args, roundMode=1)
            else:
                update_pickle(args)

        if start == -1:
            print('FEI-distributed: (setup-) Rebasing setup, but do not overwrite')
            setup(args, fullOverwrite=False)
            missingSymlinks(args, printing=False)

        if start == -2:
            print('FEI-distributed: Attempting to start a retraining')
            if args.retrain >= 0:
                if get_training_cache(args)[0] is None:
                    raise RuntimeError(
                        f'FEI-REtrain: Cannot retrain! No training data found in {args.directory}/collection/Summary.pickle')
                if args.retrain-1 > get_training_cache(args)[0]:
                    raise RuntimeError(
                        f'FEI-REtrain: Cannot retrain! Training has not reached the stage {args.retrain} yet, '
                        f'instead its at {get_training_cache(args)}!'
                    )
                if args.retrain > args.end:
                    raise RuntimeError(
                        f'FEI-REtrain: Cannot retrain if you want to end the training before the retrain stage {args.retrain}!')
                print(f'FEI-REtrain: Retraining from stage {args.retrain}')
                # 3.0/ fix pickle
                # candidates for retrain stage BDT training are computed one stage earlier
                particles, configuration = update_pickle(args, args.retrain-1, 3)
                clean_job_directory(args)
            else:
                raise RuntimeError('FEI-REtrain: Cannot retrain! No retrain stage provided!')
    else:
        # This is a new training
        # So we have to setup the whole directory (this will override any existing training)
        print('FEI-distributed: (setup+) Setup from scratch')
        setup(args, fullOverwrite=True)

    # The main loop, which steers the whole FEI training on a batch system
    # 1. We check if the FEI still requires further steps
    # 2. We do all necessary trainings which we can perform at this point in time
    # 3. We submit new jobs which will use the new trainings to reconstruct the hierarchy further
    # 4. We wait until all jobs finished
    # 5. We merge the output of the jobs
    # 6. We update the inputs of the jobs (input of next stage is the output of the current stage)
    # 7. We clean the job directories so they can be used during the next stage again.
    if get_training_cache(args)[0] is not None and get_training_cache(args)[0] > args.end:
        raise RuntimeError(
            f"FEI-distributed: Check args.end this doesn't make sense: {args.end}, training is at {get_training_cache(args)}")

    while get_training_cache(args)[1] != 2:
        if get_training_cache(args)[0] is None or get_training_cache(
                args)[0] <= 0 or get_training_cache(args)[1] == 3 or get_training_cache(args)[1] == 1:
            print('FEI-distributed: no training, because cache is None or <= 0, or roundMode is 3 or 1')
        else:
            print('FEI-distributed: (train) Do available trainings for stage: ', get_training_cache(args))
            do_trainings(args)

        print('FEI-distributed: (submit) Submitting jobs in mode: ', get_training_cache(args))
        for i in range(args.nJobs):
            clean_monitoring(args, i)
            if not submit_job(args, i):
                raise RuntimeError('Error during submitting jobs')

        print('FEI-distributed: (wait) Wait for jobs to end')
        while not jobs_finished(args):
            time.sleep(40)

        print('FEI-distributed: (merge) Merge ROOT files, with training_input.root: ', get_training_cache(args)[1])
        merge_root_files(args, get_training_cache(args)[0], roundMode=get_training_cache(args)[1])

        print('FEI-distributed: (update) Update input files with summary: ', get_training_cache(args)[1])
        update_input_files(args)

        print('FEI-distributed: (clean) Clean job directory')
        clean_job_directory(args)

        if args.once:
            break
    else:
        # This else will be called if the loop was not exited by break
        # This means the training finished and we can create our summary reports.
        print('FEI-distributed: (report) Create report')
        create_report(args)
