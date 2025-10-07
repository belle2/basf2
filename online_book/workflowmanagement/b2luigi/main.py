import b2luigi as luigi
from offlineanalysis import Plot
if __name__ == "__main__":
    output_directory = "/group/belle/users/<user>"
    luigi.set_setting("result_dir", output_directory)
    luigi.process(Plot(), workers=100)
