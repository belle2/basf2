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
    args = parser.parse_args()
    return args


def get_job_script(args, i):
    """
    Create a bash file which will be dispatched to the batch system.
    The file will run basf2 on the provided MC or the previous output
    using the provided steering file.
    """
    job_script = f"""
        if [ -f "{args.directory}/jobs/{i}/basf2_input.root" ]; then
          INPUT="{args.directory}/jobs/{i}/basf2_input.root"
        else
          INPUT="{args.directory}/jobs/{i}/input_*.root"
        fi
        time basf2 -l error {args.directory}/collection/basf2_steering_file.py -i "$INPUT" \
        -o {args.directory}/jobs/{i}/basf2_output.root &> my_output_hack.log || touch basf2_job_error
        touch basf2_finished_successfully
    """
    return job_script


def setup(args):
    """
    Setup all directories, create job_scripts, split up MC into chunks
    which are processed by each job. Create symlinks for databases.
    """
    os.chdir(args.directory)
    # Search and partition data files into even chunks
    data_files = []

    for x in args.data:
        for y in x:
            if (y.startswith("http://") or y.startswith("https://")):
                data_files += b2biiConversion.parse_process_url(y)
            else:
                data_files += glob.glob(y)
    print(f'Found {len(data_files)} MC files')
    file_sizes = []
    for file in data_files:
        file_sizes.append(os.stat(file).st_size)
    data_files_sorted = [x for _, x in sorted(zip(file_sizes, data_files))]
    n = int(len(data_files) / args.nJobs)
    if n < 1:
        raise RuntimeError(f'Too few MC files {len(data_files)} for the given number of jobs {args.nJobs}')
    data_chunks = [data_files_sorted[i::args.nJobs] for i in range(args.nJobs)]

    # Create needed directories
    print(f'Create environment in {args.directory}')
    shutil.rmtree('collection', ignore_errors=True)
    shutil.rmtree('jobs', ignore_errors=True)
    os.mkdir('collection')
    os.mkdir('collection/localdb')
    os.mkdir('jobs')
    if args.large_dir:
        if not os.path.isdir(args.large_dir):
            raise RuntimeError('Large dir does not exist. Please make sure it does.')

    shutil.copyfile(args.steering, 'collection/basf2_steering_file.py')

    for i in range(args.nJobs):
        # Create job directory
        os.mkdir(f'jobs/{i}')
        with open(f'jobs/{i}/basf2_script.sh', 'w') as f:
            f.write(get_job_script(args, i))
            os.chmod(f.fileno(), stat.S_IXUSR | stat.S_IRUSR | stat.S_IWUSR)
        # Symlink initial input data files
        for j, data_input in enumerate(data_chunks[i]):
            os.symlink(data_input, f'jobs/{i}/input_{j}.root')
        # Symlink weight directory and basf2_path
        os.symlink(args.directory + '/collection/localdb', f'jobs/{i}/localdb')


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
    os.chdir(args.directory + '/collection')
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

    ret = subprocess.call('basf2 fei/printReporting.py > ../summary.txt', shell=True)
    ret = subprocess.call('basf2 fei/latexReporting.py ../summary.tex', shell=True)
    for i in glob.glob("*.xml"):
        if not fei.core.Teacher.check_if_weightfile_is_fake(i):
            subprocess.call(f"basf2_mva_evaluate.py -id '{i[:-4]}.xml' -data 'training_input.root' "
                            f"--treename '{i[:-4]} variables' -o '../{i[:-4]}.zip'", shell=True)
    os.chdir(args.directory)
    return ret == 0


def submit_job(args, i):
    """
    Submits a job to the desired batch system.
    Currently we can run on KEKCC (long queue), KEKCC (dedicated FEI queue),
    EKP @ KIT, or your local machine
    """
    # Synchronize summaries
    if os.path.isfile(args.directory + '/collection/Summary.pickle'):
        shutil.copyfile(args.directory + '/collection/Summary.pickle', args.directory + f'/jobs/{i}/Summary.pickle')
    os.chdir(args.directory + f'/jobs/{i}/')
    if args.site == 'kekcc':
        ret = subprocess.call("bsub -q l -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kekcc2':
        ret = subprocess.call("bsub -q b2_fei -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kitekp':
        ret = subprocess.call("qsub -cwd -q express,short,medium,long -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'local':
        subprocess.Popen(['bash', './basf2_script.sh'])
        ret = 0
    else:
        raise RuntimeError(f'Given site {args.site} is not supported')
    os.chdir(args.directory)
    return ret == 0


def do_trainings(args):
    """
    Trains the multivariate classifiers for all available training data in
    the collection directory, which wasn't trained yet.
    This is called once per iteration
    """
    os.chdir(args.directory + '/collection')
    if not os.path.isfile('Summary.pickle'):
        return
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    weightfiles = fei.do_trainings(particles, configuration)
    for i in range(args.nJobs):
        for weightfile_on_disk, _ in weightfiles:
            os.symlink(args.directory + '/collection/' + weightfile_on_disk,
                       args.directory + f'/jobs/{i}/' + weightfile_on_disk)
    # Check if some xml files are missing
    xmlfiles = glob.glob("*.xml")
    for i in range(args.nJobs):
        for xmlfile in xmlfiles:
            if not os.path.isfile(args.directory + f'/jobs/{i}/' + xmlfile):
                print("Added missing symlink to ", xmlfile, " in job directory ", i)
                os.symlink(args.directory + '/collection/' + xmlfile,
                           args.directory + f'/jobs/{i}/' + xmlfile)
    os.chdir(args.directory)


def jobs_finished(args):
    """
    Check if all jobs already finished.
    Throws a runtime error of it detects an error in one of the jobs
    """
    finished = glob.glob(args.directory + '/jobs/*/basf2_finished_successfully')
    failed = glob.glob(args.directory + '/jobs/*/basf2_job_error')

    if len(failed) > 0:
        raise RuntimeError(f'basf2 execution failed! Error occurred in: {str(failed)}')

    return len(finished) == args.nJobs


def merge_root_files(args):
    """
    Merges all produced ROOT files of all jobs together
    and puts the merged ROOT files into the collection directory.
    This affects mostly
      - the training data for the multivariate classifiers
      - the monitoring files
    """
    rootfiles = []
    for f in glob.glob(args.directory + '/jobs/0/*.root'):
        f = os.path.basename(f)
        if f in ['basf2_input.root', 'basf2_output.root']:
            continue
        if f.startswith('input_'):
            continue
        # in case of training_input.root, append to already existing file
        if os.path.isfile(args.directory + '/collection/' + f) and not f == 'training_input.root':
            continue
        rootfiles.append(f)
    if len(rootfiles) == 0:
        print('There are no root files to merge')
    else:
        print('Merge the following files', rootfiles)
        for f in rootfiles:
            output = args.directory + '/collection/' + f
            inputs = [args.directory + f'/jobs/{i}/' + f for i in range(args.nJobs)]
            ret = subprocess.call(['analysis-fei-mergefiles', output] + inputs)
            if ret != 0:
                raise RuntimeError('Error during merging root files')
            # Replace mcParticlesCount.root with merged file in all directories
            # so that the individual jobs calculate the correct mcCounts and sampling rates
            if f == 'mcParticlesCount.root':
                for i in inputs:
                    os.remove(i)
                    os.symlink(output, i)


def update_input_files(args):
    """
    Updates the input files.
    For the first iteration the input files are the MC provided by the user.
    After each training this function replaces the input with the output of the previous iteration.
    Effectively this caches the whole DataStore of basf2 between the iterations.
    """
    for i in range(args.nJobs):
        output_file = args.directory + '/jobs/' + str(i) + '/basf2_output.root'
        input_file = args.directory + '/jobs/' + str(i) + '/basf2_input.root'
        if args.large_dir:
            real_input_file = args.large_dir + '/basf2_input_' + str(i) + '.root'
            shutil.move(output_file, real_input_file)
            if os.path.isfile(input_file):
                os.remove(input_file)
            os.symlink(real_input_file, input_file)
        else:
            shutil.move(output_file, input_file)
    # Saves the Summary.pickle of the first job to the collection directory
    # so we can keep track at which stage of the reconstruction the FEI is currently.
    shutil.copyfile(args.directory + '/jobs/0/Summary.pickle', args.directory + '/collection/Summary.pickle')


def clean_job_directory(args):
    """
    Cleans the job directory for the next iteration
    Meaning we remove all logs
    """
    files = glob.glob(args.directory + '/jobs/*/basf2_finished_successfully')
    files += glob.glob(args.directory + '/jobs/*/error.log')
    files += glob.glob(args.directory + '/jobs/*/output.log')
    for f in files:
        os.remove(f)


def is_still_training(args):
    """
    Checks if the FEI training is still ongoing.
    The training is finished if the FEI reached stage 7
    """
    os.chdir(args.directory + '/collection')
    if not os.path.isfile('Summary.pickle'):
        return True
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    os.chdir(args.directory)
    return configuration.cache != 7


if __name__ == '__main__':
    args = getCommandLineOptions()

    os.chdir(args.directory)

    # If the user wants resume an existing training
    # we check at which step he wants to resume and
    # try to perform the necessary steps to do so
    if args.skip:
        print('Skipping setup')
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
        else:
            raise RuntimeError(f'Unknown skip parameter {args.skip}')

        if start == 7:
            create_report(args)
            sys.exit(0)

        # The user wants to submit the jobs again
        if start >= 5:
            print('Submitting jobs')
            for i in range(args.nJobs):
                # The user wants to resubmit jobs, this means the training of some jobs failed
                # We check which jobs contained an error flag, and were not successful
                # These jobs are submitted again, other jobs are skipped (continue)
                if start >= 6:
                    error_file = args.directory + f'/jobs/{i}/basf2_job_error'
                    success_file = args.directory + f'/jobs/{i}/basf2_finished_successfully'
                    if os.path.isfile(error_file) or not os.path.isfile(success_file):
                        print(f"Delete {error_file} and resubmit job")
                        if os.path.isfile(error_file):
                            os.remove(error_file)
                        if os.path.isfile(success_file):
                            os.remove(success_file)
                    else:
                        continue
                # Reset Summary file
                shutil.copyfile(os.path.join(args.directory, 'collection/Summary.pickle'),
                                os.path.join(args.directory, f'jobs/{i}/Summary.pickle'))
                if not submit_job(args, i):
                    raise RuntimeError('Error during submitting job')

        if start >= 4:
            print('Wait for jobs to end')
            while not jobs_finished(args):
                time.sleep(40)

        if start >= 3:
            print('Merge ROOT files')
            merge_root_files(args)

        if start >= 2:
            print('Update input files')
            update_input_files(args)

        if start >= 1:
            print('Clean job directory')
            clean_job_directory(args)

    else:
        # This is a new training
        # So we have to setup the whole directory (this will override any existing training)
        setup(args)

    # The main loop, which steers the whole FEI training on a batch system
    # 1. We check if the FEI still requires further steps
    # 2. We do all necessary trainings which we can perform at this point in time
    # 3. We submit new jobs which will use the new trainings to reconstruct the hierarchy further
    # 4. We wait until all jobs finished
    # 5. We merge the output of the jobs
    # 6. We update the inputs of the jobs (input of next stage is the output of the current stage)
    # 7. We clean the job directories so they can be used during the next stage again.
    while is_still_training(args):
        print('Do available trainings')
        do_trainings(args)

        print('Submitting jobs')
        for i in range(args.nJobs):
            if not submit_job(args, i):
                raise RuntimeError('Error during submitting jobs')

        print('Wait for jobs to end')
        while not jobs_finished(args):
            time.sleep(40)

        print('Merge ROOT files')
        merge_root_files(args)

        print('Update input files')
        update_input_files(args)

        print('Clean job directory')
        clean_job_directory(args)

        if args.once:
            break
    else:
        # This else will be called if the loop was not existed by break
        # This means the training finished and we can create our summary reports.
        create_report(args)
