To train create new weight files for the MVA quality estimator
use the command: (which runs the script)

python3 combined_quality_estimator_teacher.py --batch

The "--batch" is needed since the script uses b2luigi.

To configure the number of training/testing events, experiment number
and background files, see the "settings.json" file. It is recommended
to configure this file before running the code.

--------------------------------

The most important settings are:

n_events_training: which decides how many events that will be
used for training the MVA:s.

n_events_testing: which decides how many events that are used
for testing the performance of the weightfiles.

n_events_per_task: the script generates events for training and testing.
By creating many small batches one can speedup the event generation
with parallel processing. This parameter determines the size of each
mini-batch.

workers: determines the maximum number of parallel process the script can
perform. This needs to be higher than the total number of tasks. Keeping
it at >100 should suffice.

bkgfiles_by_exp: this is the path to the background files that are used for
the background overlay. This needs to be kept up to date.

--------------------------------

The script is divided into classes. Roughly in their order of execution,
they perform the following tasks:

GenerateSimTask: each call generates a fraction of the configured total number of
events.

SplitNMergeSimTask: calls the GenerateSimTask and then combines the output files
into one large file for the training and testing tasks.

CheckExistingFile: this checks if a file exists. Each class is considered finished
and successful if the file exists.

VXDQEDataCollectionTask: imports events and performs VXD-only tracking to generate
training data for the VXD QE MVA.

CDCQEDataCollectionTask: imports events and performs CDC-only tracking to generate 
training data for the CDC QE MVA.

Note: a BaseTask has a general structure that lets CDC-, VXD- and Reco-tasks inherit said 
structure. By making small modifications it is possible to adapt the class for
each type of tracking.

TrackQETeacherBaseTask: this task takes in the outputs from different
QEDataCollectionTasks and trains a QE MVA. The output is a local database containing
the weightfile.

RecoTrackQEDataCollectionTask: imports events and weightfiles to perform full CDC
and VXD tracking to generate training data for the RecoTrack QE MVA. This task
requires both the VXD and CDC weightfiles from their TeacherTasks.

TrackQEEvaluationBaseTask: this task uses the testing events to examine the trained
MVAs. It outputs a .zip file containing all evaluation plots.

HarvestingValidationBaseTask: this code uses the HarvestingValidation tracking package
in basf2 to examine the performance of the new weightfiles in regards to finding
efficiency, clone rate and fake rate.

PlotsFromHarvestingValidationBaseTask: this uses the output from the 
HarvestingValidationTask to make plots of the performance as a .pdf
file.

QEWeightsLocalDBCreator: this task imports the weightfiles and produces a payload.
Currently, this task is not used since the Teacher task handles creations of
payloads.

MasterTask: acts as a wrapper task and is the only one explicitly called by the
script. In order for it to finish all other tasks must successfully finish.

--------------------------------

To debug the code, open the file referenced by the red text in the b2luigi output.
Both a stderr and a stdout is generated. It may be neccesary to inspect both,
especially if the stderr file is empty.

--------------------------------

You can use the "boards.py" script to create a local payload for disabling boards.
The payload needs to be prepended. Which boards are disabled is configured by the
"CDCBadBaords_example.txt" file.

On each row you write the index of a board and its efficiency.
"24 0.0" means that board 24 will have a 0% effciency ie it will be turned
off. An example of what this file can look like is:

12 0.0
14 0.5
47 0.0
48 0.0
6 0.0
100 0.0

Note: this code was developed for release-10, and newer releases may provide
different methods for disabling boards.
