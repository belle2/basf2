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
* scripts should not depend on the working directory being the current directory
* scripts should put the output to be kept in OUTPUT_DIR, all intermediate files
  in the current working directory when the script is started
* scripts should indicate failure with a non-zero return code.
* stdout and stderr of all scripts will automatically be saved to `OUTPUT_DIR`

The validation can then be executed with `./run.sh` which takes two optional arguments

* the stage, either "manual" or "automatic"
* the output directory where to store files to be kept (which will then be
  accessible as `OUTPUT_DIR`)

```
./run.sh automatic automatic-validation-release-04-02-06
```
