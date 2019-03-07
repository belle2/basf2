This is a set of scripts used to update the standard charged particle lists working points during early validation of new releases, as well as to check their performance. They must be run before physics and skimming validation as those depend on functions contained in stdCharged.py.

1. Producing the ntuples
After setting up the desired release environment, run:

basf2 truth_charged.py inputfile outputfile

Where inputfile is one of the MC samples produced for validation (usually ccbar BGx1) and outputfile is your desired output root file. If no output is specified, it will be saved as truth_charged.root

2. Extracting the working points

Run the stdCharged.ipynb notebook. If you did not use the default output in step 1, you will need to fill that in. 
The script will output a set of working points as a 3x5 matrix; open analysis/scripts/stdCharged.py and replace the old working points with it.
The script also produces ROC plots for each of the five PID, as well as confusion/purity matrices for validation purposes.