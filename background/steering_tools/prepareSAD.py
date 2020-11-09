import subprocess

# this scripts prepares SAD files for G4 simulation
# merges all files for individual source, cuts -4m < s < 4m

bkgs = ("LERT", "LERC", "LERB", "HERT", "HERC", "HERB")
bkgs1 = ("Touschek_LER", "Coulomb_LER", "Brems_LER", "Touschek_HER", "Coulomb_HER", "Brems_HER")
mainDir = "/group/belle2/BGcampaigns/SAD"  # location of input SAD files
dirr = "20191207_1X80-60_BG_study"         # subdir of input SAD files
outFile = "20191207_1X80-60_BG_study"      # name of the output SAD file which is created into to the "mainDir/forG4/"

i = 0
for bkg in bkgs:
    com = [
        "root",
        "-l",
        "-b",
        "-q",
        f'background/steering_tools/prepareSADsample.C("{mainDir}/{dirr}/{bkg}*","{mainDir}/forG4/{bkgs1[i]}_{outFile}.root")']
    with open(f"{mainDir}/forG4/{bkgs1[i]}_{outFile}_log.txt", "w") as log:
        subprocess.check_call(com, stdout=log)

    print(com)
    i = i+1
