# Examples

If you are new to ``basf2``, a good starting points are the tutorials in the ``tutorials`` subdirectory.

## Examples data dir

Several of the scripts here use input files from the examples data directory.
This directory is **not** contained in this repository itself. 
If everything is set up correctly, then

```sh
echo $BELLE2_EXAMPLES_DATA_DIR
```

should point to this directory. If the output of this command is blank, then you're
probably missing it.
You can copy the example data from kekcc (check the output of the above command
on kekcc after setting up basf2 from cvmfs there to locate it) to your machine
and then do

```sh
export BELLE2_EXAMPLES_DATA_DIR="/path/to/your/example/dir"
```

Before you start basf2 or (even better) add it to your ``~/.bashrc`` so this setting
persists.

