from dft import steering_training_data
import os
import subprocess
import pickle

# Basic tutorial for training of the deep flavor tagger.
# make sure you have installed Theano (pip3 install theano),
# pylearn2 and CUDA > v7.5 (lower CUDA versions will work though,
# but not automatically due to the gcc version in the basf2 environment)

# you also might need pandas, nose_parametriezed, root-numpy

# chose filenames. standard training is written for monogeneric monte carlo. a decay into nu_tau and anti-nu_tau
# feel free to adapt the steering file, or get inspired by test_expert_jpsi for your own specific case, if necessary

# enter a list of strings
filenames = []

# name of the identifier in the database
uniqueIdentifier = 'standard'

# None uses standard parameters, insert here a list of basf2 variable names, if needed
variable_list = None

# use current directory as working directory
working_dir = ''

# path of the extern train command will be
extern_train_command_path = os.path.join(working_dir, uniqueIdentifier + '_teacher_prefix')

# write out the variable file, and the training command for extern training
# internal training could be implemented, too

# if needed you can pass keyword-arguments for the trained mlp,
# for example the number of 8 hidden layers and uniform weight intialization
# classifier_args = {'nhid': 8, 'weight_init':'uniform'}

steering_training_data.create_train_data(
    working_dir,
    filenames,
    uniqueIdentifier,
    variable_list,
    convert_to_cpu=False,
    overwrite=True)

# run the external bash command, just with ./bash standard_teacher_prefix
# <prefix>_teacher_prefix

# or execute via subprocess
# or rewrite the function, just as described in mva/tutorials
with open(extern_train_command_path, 'r') as f:
    command = f.readlines()[0]
    print(command)
# start the actuall training
job = subprocess.Popen(command, shell=True)
job.wait()


####################################
# Basic tutorial for the expert

# after the training, you can use the expert with

# basf2_mva_expert --datafile <training_data_file> --treename dft_variables
# --weightfile <uniqueIdentifier> --outputfile <outputfile>.root

# make sure the data_file is in the same format as the training data
# for example, create it with create_train_data (maybe a function for just creating datafiles will be added in future)

# or just look at the example for usage inside the basf2 framework
steering_training_data.test_expert(working_dir, filenames, uniqueIdentifier)
