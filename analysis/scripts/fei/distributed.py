#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import shutil
import subprocess
import sys
import os
import argparse
import glob
import time
import stat
import shutil
import pickle

import fei


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
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
                        help='Data files in bash expansion syntax')
    parser.add_argument('-x', '--skip-to', dest='skip', type=str, default='',
                        help='Skip setup of directories')
    parser.add_argument('-o', '--once', dest='once', action='store_true',
                        help='Execute just once time, instead of waiting until a Summary is produced.')
    parser.add_argument('-s', '--site', dest='site', type=str, default='kekcc',
                        help='Site to use [kekcc|kitekp|local]')
    args = parser.parse_args()
    return args


def get_job_script(args, i):
    job_script = """
        if [ -f "{d}/basf2_input.root" ]; then
          INPUT="{d}/basf2_input.root"
        else
          INPUT="{d}/input_*.root"
        fi
        time basf2 -l error {d}/collection/basf2_steering_file.py -i "$INPUT" -o {d}/jobs/{i}/basf2_output.root {pipes}
        touch basf2_finished_successfully
    """.format(d=args.directory, i=str(i), pipes="&> my_output_hack.log || touch basf2_job_error")
    return job_script


def setup(args):
    os.chdir(args.directory)
    # Search and partition data files into even chunks
    data_files = []
    for x in args.data:
        for y in x:
            data_files += glob.glob(y)
    print('Found {} MC files'.format(len(data_files)))
    n = int(len(data_files) / args.nJobs)
    if n < 1:
        raise RuntimeError('Too few MC files {} for the given number of jobs {}'.format(len(data_files), args.nJobs))
    data_chunks = [data_files[i:i + n] for i in range(0, len(data_files), n)]

    # Create needed directories
    print('Create environment in {}'.format(args.directory))
    shutil.rmtree('collection', ignore_errors=True)
    shutil.rmtree('jobs', ignore_errors=True)
    os.mkdir('collection')
    os.mkdir('collection/localdb')
    os.mkdir('collection/B2BII_MC_database')
    os.mkdir('jobs')
    if args.large_dir:
        os.mkdir(args.large_dir)

    for i in range(args.nJobs):
        # Create job directory
        os.mkdir('jobs/{}'.format(i))
        # Create job script
        with open('jobs/{}/basf2_steering_file.py'.format(i), 'w') as f:
            f.write(get_steering_file(args, i))
            os.chmod(f.fileno(), stat.S_IXUSR | stat.S_IRUSR)
        with open('jobs/{}/basf2_script.sh'.format(i), 'w') as f:
            f.write(get_job_script())
            os.chmod(f.fileno(), stat.S_IXUSR | stat.S_IRUSR)
        # Symlink initial input data files
        for j, data_input in enumerate(data_chunks[i]):
            os.symlink(data_input, 'jobs/{}/input_{}.root'.format(i, j))
        # Symlink weight directory and basf2_path
        os.symlink(args.directory + '/collection/localdb', 'jobs/{}/localdb'.format(i))
        os.symlink(args.directory + '/collection/B2BII_MC_database', 'jobs/{}/B2BII_MC_database'.format(i))


def create_report(args):
    os.chdir(args.directory + '/collection')
    ret = subprocess.call('basf2 fei/printReporting.py > ../summary.txt', shell=True)
    ret = subprocess.call('basf2 fei/latexReporting.py ../summary.tex', shell=True)
    for i in glob.glob("*.xml"):
        subprocess.call("basf2_mva_evaluate.py -id '{i}.xml' -data '{i}.root' "
                        "--treename variables -o '../{i}.pdf'".format(i=i[:-4]),
                        shell=True)
    os.chdir(args.directory)
    return ret == 0


def submit_job(args, i):
    os.chdir(args.directory + '/jobs/{}/'.format(i))
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
        raise RuntimeError('Given site {} is not supported'.format(args.site))
    os.chdir(args.directory)
    return ret == 0


def do_trainings(args):
    os.chdir(args.directory + '/collection')
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    fei.do_trainings(particles, configuration)
    os.chdir(args.directory)


def jobs_finished(args):
    finished = glob.glob(args.directory + '/jobs/*/basf2_finished_successfully')
    failed = glob.glob(args.directory + '/jobs/*/basf2_job_error')

    if len(failed) > 0:
        raise RuntimeError('basf2 execution failed! Error occurred in: {}'.format(str(failed)))

    return len(finished) == args.nJobs


def merge_root_files(args):
    rootfiles = []
    for f in glob.glob(args.directory + '/jobs/0/*.root'):
        f = os.path.basename(f)
        if f in ['basf2_input.root', 'basf2_output.root']:
            continue
        if f.startswith('input_'):
            continue
        if os.path.isfile(args.directory + '/collection/' + f):
            continue
        rootfiles.append(f)
    if len(rootfiles) == 0:
        print('There are no root files to merge')
    else:
        print('Merge the following files', rootfiles)
        for f in rootfiles:
            output = args.directory + '/collection/' + f
            inputs = glob.glob(args.directory + '/jobs/*/' + f)
            ret = subprocess.call(['fei_merge_files', output] + inputs)
            if ret != 0:
                raise RuntimeError('Error during merging root files')


def update_input_files(args):
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
    shutil.copyfile(args.directory + '/jobs/1/Summary.pickle', args.directory + '/collection/Summary.pickle')


def clean_job_directory(args):
    files = glob.glob(args.directory + '/jobs/*/basf2_finished_successfully')
    files += glob.glob(args.directory + '/jobs/*/error.log')
    files += glob.glob(args.directory + '/jobs/*/output.log')
    for f in files:
        os.remove(f)


def is_still_training(args):
    os.chdir(args.directory + '/collection')
    if not os.path.isfile('Summary.pickle'):
        return True
    particles, configuration = pickle.load(open('Summary.pickle', 'rb'))
    os.chdir(args.directory)
    return configuration.cache != 7


if __name__ == '__main__':
    args = getCommandLineOptions()

    os.chdir(args.directory)

    print("Copy steering file into collection directory")
    shutil.copyfile(args.steering, 'collection/basf2_steering_file.py')

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
        elif args.skip == 'run':
            start = 0
        else:
            raise RuntimeError('Unkown skip parameter {}'.format(args.skip))

        if start >= 5:
            print('Submitting jobs')
            for i in range(args.nJobs):
                if start >= 6:
                    error_file = args.directory + '/jobs/{}/basf2_job_error'.format(i)
                    success_file = args.directory + '/jobs/{}/basf2_finished_successfully'.format(i)
                    if os.path.isfile(error_file) or not os.path.isfile(success_file):
                        print("Delete " + error_file + " and resubmit job")
                        if os.path.isfile(error_file):
                            os.remove(error_file)
                        if os.path.isfile(success_file):
                            os.remove(success_file)
                    else:
                        continue
                if not submit_job(args, i):
                    raise RuntimeError('Error during submiting job')

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
        setup(args)

    while is_still_training():
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
        create_report(args)
