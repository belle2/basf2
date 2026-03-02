import os
import b2luigi
import numpy as np
import json
from sampler_b2bii import main as sampler_main

OUTPATH = '/group/belle2/group/physics/TDCPV/TFlaT_Belle/Test_Submission/'

DICO_DATA = {}
CHUNKSIZE = 1


def get_chunked_array(filename):
    with open(filename) as f:
        tab = json.load(f)
    return np.array_split(tab, len(tab)/CHUNKSIZE)


# Declare Input Files
DICO_DATA["TFLaT_Files_Test"] = get_chunked_array(
    "/group/belle2/group/physics/TDCPV/TFlaT_Belle/Simulated_Samples/Input_Files_test.json")
# DICO_DATA["TFLaT_Files"] = get_chunked_array("/group/belle2/group/physics/TDCPV/TFlaT_Belle/Simulated_Samples/Input_Files.json")


class RecoTask(b2luigi.Task):

    queue = "l"

    key = b2luigi.Parameter()
    num = b2luigi.IntParameter()

    def output(self):
        return self.add_to_output(f"chunk_{self.num}/done.txt")

    def run(self):
        mynum = self.num
        mykey = self.key

        unique_id = "TFlaT_Belle_light_2601_hyperion"

        workdir = os.path.join(OUTPATH, f"key={mykey}/num={mynum}/")
        os.makedirs(workdir, exist_ok=True)

        sampler_main(unique_id, DICO_DATA[mykey][mynum].tolist(), working_dir=workdir)


class Wrapper(b2luigi.WrapperTask):
    def requires(self):
        for key in DICO_DATA.keys():
            for num in range(0, len(DICO_DATA[key])):  # DICO_DATA[key]
                yield RecoTask(key=key, num=num)


if __name__ == '__main__':
    # Choose htcondor or local as our batch system
    b2luigi.set_setting("batch_system", 'lsf')  # 'local')

    # Setup the correct environment on the workers
    b2luigi.set_setting("env_script", "setenv.sh")

    # Most likely your executable from the submission node is not the same on
    # the worker node, so specify it explicitly
    b2luigi.set_setting("executable", ["python3"])

    b2luigi.set_setting("result_dir", OUTPATH)
    b2luigi.set_setting("log_dir", OUTPATH)

    b2luigi.process(Wrapper(), batch=True, workers=3000)
