HLT Release Validation
======================

This directory contains all the scripts to perform the validation of a given
release to work on the HLT (semi-)automatically on the [CI
system](https://bamboo.desy.de/browse/B2-HLTVALID/branches). There are two
separate stages:

1. an automatic stage which is supposed to be executed automatically by the CI
   on the release branch whenever a PR is merged.
2. a manual stage which can be triggered to run on the CI once the automatic
   stage is done to perform additional, potential long running validation before
   tagging a release.

For both stages the CI will take all scripts in this folder starting with
`automatic-` or `manual-` and execute them in alphabetical order. Files ending
in `.py` that are **not executable** will be executed with basf2, all other files
will just be executed and depend on a proper shebang and **need to be executable**.

* scripts are only executed if all prior scripts were successful so they can
  depend on the output files from prior scripts
* scripts can depend on the environment variables `BELLE2_RELEASE_DIR`,
  `BELLE2_VALIDATION_DIR`, `RELEASE`, `OUTPUT_DIR`, and `NCPUS` being set
* scripts can depend on `OUTPUT_DIR` and `BELLE2_VALIDATION_DIR` being an existing directory
* scripts can depend on `VALIDATION_EXP`, `VALIDATION_RUN`, `VALIDATION_RAWDATA` to be set
  and contain exp and run number and a raw file from a (hopefully) recent run.
* scripts should not depend on the working directory being the current directory
* scripts should put the output to be kept in `OUTPUT_DIR`, all intermediate files
  in the current working directory when the script is started
* scripts should indicate failure with a non-zero return code.
* stdout and stderr of all scripts will automatically be saved to `OUTPUT_DIR`

How to Run
----------

The validation can then be executed with `./run.sh` which takes two optional arguments

* the stage, either "manual" or "automatic"
* the output directory where to store files to be kept (which will then be
  accessible as `OUTPUT_DIR`)

```
./run.sh automatic automatic-validation-release-04-02-06
```

How to Update the Validation Run
--------------------------------

Copy around 10k events from the raw data of the latest non-filtered physics run
to the [DESY cloud](https://syncandshare.desy.de/) validation data account into
the hltvalidation folder and update the corresponding `info.sh` file in that folder.
Note that the `ROIs` **branch needs to be removed** from the original raw file.

The tool `b2file-mix` can be used to sample events from the raw data root files
and remove the branch at the same time:

```
b2file-mix -n 10000 --exclude ROIs --keep-eventinfo -o output.root [inputfiles...]
```

Now the validation data needs to be updated, either locally with `b2install-data
validation` or on bamboo by manually running the [B2-CLEAN](https://bamboo.desy.de/browse/B2-CLEAN)
build.

When to Run
-----------

These tests are not intended to run on each individual PR to the main (master)
branch, but only for release version which are intended to run on the HLT.
This should prevent release tags which are not usable on the HLT system.

We have a bamboo build for this: [B2-HLTVALID](https://bamboo.desy.de/browse/B2-HLTVALID0).
It doesn't run fully automatic but once a branch plan is created it will run
automatically for each new commit on that branch if the normal build is
successful.

But it will only run the first, automatic stage for each commit. Once that is
checked the second stage can be started manually to do a more thorough check.

If new payloads are requested to be included in the online globaltag the build can
be triggered manually. See the section on the Conditions Database.

What to check
-------------

The validation build will store the output of all log files as artifacts. On the
build log click on the artifacts tab and select the artifact
`automatic_validation_result` to see the content. For the second stage this will
be the `final_validation_result`.

* There shouldn't be a large amount of errors.
* The proportional memory result should be below 2GB per core. We have 16 cores
  on bamboo.
* With 16 cores the runtime should stay around ~500s for 10k events. Small
  changes are expected just from possible load on the system. This doesn't
  necessarily scale linearly with the number of cores as there's a large setup
  time in the beginning which stays constant. Also for large number of cores
  there's extra overhead.
* The trigger output should look sensible compared to previous results.


Conditions Database
-------------------

The validation runs on a custom set of conditions payload specially created for
each validation run. This is done for the following reasons:

* We want to test the release and any payloads requested for inclusion in the online tag
* We might actually need the new payloads for a new release
* We should stay to the original payloads used then taking the run if possible.
* The software might even depend on payloads already in online but which were not
  active when the run was taken.

So for each validation run we

1. take all the recent versions of payload from the online gt (the ones still
   valid for new runs)
2. we replace them with the ones of the same name that were used when recording
   the run if those are different
3. we put everything from `staging_online` on top of that: payloads in staging
   replace payloads used when recording the run.
4. we create a local database to use for the validation.

This should keep the payloads as close as possible to the conditions when taking
data while still allowing some validation. For this to work the validation run
should be updated frequently just to keep the differences minimal.

The results will contain a log of which payloads where choosen in which revision
and where they were taken from. If too many payloads are taken from the run
number there are many changes between the current payloads and the conditions
back then and the validation run should probably be updated.

If one wants to prepare a special globaltag for validation one can set the
environment variable `VALIDATION_GLOBALTAG`. In this case exactly the payloads
from this globaltag will be downloaded.

In the bamboo build a custom globaltag can be used by setting the build variable
`custom_globaltag` to a non-empty value. Either for all runs by modifying the
build or branch settings or by running a single custom build with Run -> "Run
customized..."
