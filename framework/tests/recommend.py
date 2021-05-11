import basf2 as b2
import subprocess

subprocess.run(['b2conditionsdb-recommend'], check=True)

subprocess.run(['b2conditionsdb-recommend', '--oneline'], check=True)

subprocess.run(['b2conditionsdb-recommend', b2.find_file('analysis/tests/mdst.root')], check=True)
