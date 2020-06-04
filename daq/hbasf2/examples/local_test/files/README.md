# Create Input Files

For the tests in this folder (and also for other tests), you need to create input files.
For this, use a recorded data root file, e.g. from KEKCC.

Then execute the following:

     b2hlt_prepareRawInputFile.py <input> <output>

By default, this will create three files `<output>.root`, `<output>.sroot` and `<output>.raw` with the first 10 events of the input file in a format viable for HLT tests.
For express reco tests, you need to add the option `--expressreco`:

     b2hlt_prepareRawInputFile.py <input> <output> --expressreco

If you want to extract more events, use the `--number` option.
For most (if not all) of the tests, the raw-file is needed.
