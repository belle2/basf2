#!/usr/bin/env python
# -*- coding: utf-8 -*-

import shutil
import subprocess
import sys
import os
import argparse
import glob
import time
import stat


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--steeringFile', dest='steering', type=str, required=True,
                        help='Steering file')
    parser.add_argument('-w', '--workingDirectory', dest='directory', type=str, required=True,
                        help='Directory file')
    parser.add_argument('-n', '--nJobs', dest='nJobs', type=int, default=100,
                        help='Number of jobs')
    parser.add_argument('-d', '--data', dest='data', type=str, required=True, action='append', nargs='+',
                        help='Data files in bash expansion syntax')
    parser.add_argument('-x', '--skip-setup', dest='skip', action='store_true',
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
        time basf2 -l error --execute-path basf2_path.pickle -i "$INPUT" -o {d}/basf2_output.root {pipes}
        touch basf2_finished_successfully
    """.format(d=args.directory + '/jobs/' + str(i), pipes="&> my_output_hack.log || touch basf2_job_error")
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
    data_chunks = [data_files[i:i+n] for i in range(0, len(data_files), n)]

    # Create needed directories
    print('Create environment in {}'.format(args.directory))
    shutil.rmtree('collection')
    shutil.rmtree('jobs')
    os.mkdir('collection')
    os.mkdir('collection/weights')
    open('collection/basf2_path.pickle', 'w').close()
    os.mkdir('jobs')

    for i in range(args.nJobs):
        # Create job directory
        os.mkdir('jobs/{}'.format(i))
        # Create job script
        with open('jobs/{}/basf2_script.sh'.format(i), 'w') as f:
            f.write(get_job_script(args, i))
            os.chmod(f.fileno(), stat.S_IXUSR | stat.S_IRUSR)
        # Symlink initial input data files
        for j, data_input in enumerate(data_chunks[i]):
            os.symlink(data_input, 'jobs/{}/input_{}.root'.format(i, j))
        # Symlink weight directory and basf2_path
        os.symlink(args.directory + '/collection/weights/', 'jobs/{}/weights'.format(i, j))
        os.symlink(args.directory + '/collection/basf2_path.pickle', 'jobs/{}/basf2_path.pickle'.format(i, j))


def run_basf2(args):
    os.chdir(args.directory + '/collection')
    if args.site == 'kekcc':
        ret = subprocess.call(['basf2', args.steering, '--dump-path', 'basf2_path.pickle', '-i', 'dummy.root', '--', '-monitor',
                               '--verbose', '--nThreads', '4', '--cache', 'cache.pkl', '--externTeacher', 'externClusterTeacher'])
    else:
        ret = subprocess.call(['basf2', args.steering, '--dump-path', 'basf2_path.pickle', '-i', 'dummy.root', '--',
                               '--verbose', '--nThreads', '4', '--cache', 'cache.pkl', '-monitor', '-prune'])
    os.chdir(args.directory)
    return ret == 0


def dump_final_fei_path(args):
    os.chdir(args.directory + '/collection')
    ret = subprocess.call(['basf2', args.steering, '--dump-path', 'basf2_final_path.pickle', '-i', 'dummy.root', '--',
                           '--verbose', '--dump-path'])
    shutil.copy2('basf2_final_path.pickle', 'basf2_path.pickle')
    os.chdir(args.directory)
    return ret == 0


def create_report(args, summary_file):
    os.chdir(args.directory + '/collection')
    if len(summary_file) > 1:
        raise RuntimeError('Need exactly one summary file to procede found {}'.format(len(summary_file)))
    ret = subprocess.call('basf2 fei/printReporting.py {} > ../summary.txt'.format(summary_file[0]), shell=True)
    ret = subprocess.call('basf2 fei/latexReporting.py {} ../summary.tex'.format(summary_file[0]), shell=True)
    os.chdir(args.directory)
    return ret == 0


def submit_job(args, i):
    to_symlink = glob.glob(args.directory + '/collection/*.config')
    os.chdir(args.directory + '/jobs/' + str(i))
    for f in to_symlink:
        link = os.path.basename(f)
        if not os.path.islink(link):
            os.symlink(f, link)

    if args.site == 'kekcc':
        ret = subprocess.call("bsub -q s -e error.log -o output.log ./basf2_script.sh | cut -f 2 -d ' ' | sed 's/<//' | sed 's/>//' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'kitekp':
        ret = subprocess.call("qsub -cwd -q express,short,medium,long -e error.log -o output.log -V basf2_script.sh | cut -f 3 -d ' ' > basf2_jobid", shell=True)  # noqa
    elif args.site == 'local':
        subprocess.Popen(['bash', './basf2_script.sh'])
        ret = 0
    else:
        raise RuntimeError('Given site {} is not supported'.format(args.site))
    os.chdir(args.directory)
    return ret == 0


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
        path = args.directory + '/jobs/' + str(i)
        shutil.move(path + '/basf2_output.root', path + '/basf2_input.root')


def clean_job_directory(args):
    files = glob.glob(args.directory + '/jobs/*/basf2_finished_successfully')
    files += glob.glob(args.directory + '/jobs/*/error.log')
    files += glob.glob(args.directory + '/jobs/*/output.log')
    for f in files:
        os.remove(f)


if __name__ == '__main__':
    args = getCommandLineOptions()

    os.chdir(args.directory)

    if args.skip:
        print('Skipping setup')
    else:
        setup(args)

    while True:
        print('Run basf2')
        if not run_basf2(args):
            raise RuntimeError('Error during executing basf2')

        summary_file = glob.glob(args.directory + '/collection/Summary_*.pickle')
        if len(summary_file) > 0:
            dump_final_fei_path(args)
            create_report(args, summary_file)
            break

        print('Submitting jobs')
        for i in range(args.nJobs):
            if not submit_job(args, i):
                raise RuntimeError('Error during submiting job')

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
