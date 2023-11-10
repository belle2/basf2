## MASTERCLASS UPDATE

# 1 BUGFIX

In **/basf2/masterclass/dataobjects/src/BEvent.cc** there was a ROOT based bug in **BEvent::AddTrack** function

# 2 Additional test

Created new test **/basf2/masterclass/tests/masterclass_output.py** in order to realize if new ROOT based bugs arise in future releases.

# 3 Updating steering file & New Dataset

The main example **/basf2/masterclass/examples/masterclass.py** doesnt filter any data at all.
For this purposes I first increased muon efficiency in **/basf2/masterclass/modules/src/MasterClassModules.cc**.
Next I updated the original steering file example to cut based on skims from the software trigger.
For the dataset we decided to use:

**/belle/Data/release-06-00-08/DB00000498/PromptExp26/prod00029657/e0026/4S/r00000/mdst/sub00**

and applying cuts on skims for: 
b_neutral, b_charged, dstar_1, dstar_2, dstar_3, dstar_4, jpsi, kshort,

example of running steering file:
basf2 -n100 -i "/group/belle/users/kspenko/data/masterclass/exp26/sub00/*.root" masterclass.py

# 4 Complications

Becuase bugfix was not corrected in any release we resorted to running grid jobs with local copy of the compiled module.
The command for running on grid looked like this:

gbasf2 -p masterclass2 -s prerelease-08-00-00a masterclass.py -i /belle/Data/release-06-00-08/DB00000498/PromptExp26/prod00029657/e0026/4S/r00000/mdst/sub00 -f="libmasterclass_modules.so, libmasterclass_modules.b2modmap, libmasterclass_dataobjects.so" --platform EL7 --basf2opt='-n 1000'

or alternatively on a smaller dataset specified by LPN, that you get by running:

gbasf2 -p masterclass02 -s release-06-01-12 masterclass2.py --input_dslist LPN_masterclass.list -f="libmasterclass_modules.so, libmasterclass_modules.b2modmap, libmasterclass_dataobjects.so"

gb2_ds_list /belle/Data/release-06-00-08/DB00000498/PromptExp26/prod00029657/e0026/4S/r00000/mdst/sub00

# 5 Steps for preparing localy compiled module

If you want to precompile module to pass to grid, it is IMPORTANT to note,
it HAS to be compiled on the same release you want it to run on the grid!

checkout the release, overwrite module with your changes and copy the compiled libraries from
/basf2/modules/Linux../opt/








scp ~/basf2/modules/Linux_x86_64/opt/libmasterclass_modules.b2modmap .
scp ~/basf2/modules/Linux_x86_64/opt/libmasterclass_modules.so .
scp ~/basf2/lib/Linux_x86_64/opt/libmasterclass_dataobjects.so .

export PATH=~/bin:$PATH
export LD_LIBRARY_PATH=~/masterclass:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
