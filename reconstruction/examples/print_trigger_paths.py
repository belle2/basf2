from basf2 import *
from reconstruction import add_reconstruction

modes = ["fast_reco", "hlt", "all"]
labels = ["Fast Reco Trigger", "HLT Trigger", "All (default)"]

for mode, label in zip(modes, labels):
    print("=" * 30)
    print("%s Mode:" % label)
    path = create_path()
    add_reconstruction(path, trigger_mode=mode, match_to_mc_information=False)
    print(path)
