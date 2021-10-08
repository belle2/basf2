import re
import difflib
import sys
import basf2

start = '--> Begin'
end = '<-- End'
templatestring = "# Decay {particle}. Copy/paste into {mcsample}RARE_BELLE2.DEC after each update."

particle_mc_match = {'mixed': {'particle': 'B0Rare', 'antiparticle': 'anti-B0Rare'},
                     'charged': {'particle': 'B+Rare', 'antiparticle': 'B-Rare'}}


decay_belle2 = {}

with open(basf2.find_file('decfiles/dec/DECAY_BELLE2.DEC')) as decfile:
    for mcsample in ['mixed', 'charged']:
        for particle in particle_mc_match[mcsample].values():
            formatted_string = templatestring.format(particle=particle,
                                                     mcsample=mcsample.upper())
            startstring = formatted_string + ' '*(87-len(formatted_string) - len(start)) + start
            endstring = formatted_string + ' '*(87-len(formatted_string) - len(end)) + end

            read = False
            for lineno, line in enumerate(decfile):
                if startstring in line:
                    decay_belle2[particle] = []
                    read = True
                elif read and endstring in line:
                    break
                elif read and not (line.startswith('#') or line == '\n'):
                    decay_belle2[particle].append((lineno, line))
            else:
                print("Couldn't find end string in DECFILE, something must be wrong")

special_files = {}
re_decay = re.compile('Decay .*B([^ ]+).*\n')
re_enddecay = re.compile('Enddecay *')

for mcsample in ['mixed', 'charged']:
    with open(basf2.find_file('decfiles/dec/' + mcsample.upper() + 'RARE_BELLE2.DEC')) as decfile:
        for particle in particle_mc_match[mcsample].values():
            read = False
            for lineno, line in enumerate(decfile):
                if re_decay.match(line):
                    special_files[particle] = []
                    read = True
                elif read and re_enddecay.match(line):
                    break
                elif read and not (line.startswith('#') or line == '\n'):
                    special_files[particle].append((lineno, line))
            else:
                print(f"Couldn't find Enddecay in special file {mcsample.upper()} + 'RARE_BELLE2.DEC for "
                      f"particle {particle}, something must be wrong")

diff_found = False
differ = difflib.Differ()
for particle in decay_belle2.keys():
    for group in (d for d in difflib.SequenceMatcher(None, [line for _, line in decay_belle2[particle]],
                                                           [line for _, line in special_files[particle]])
                  .get_grouped_opcodes(0)):
        for change in group:
            if change[0] == 'equal':
                continue
            else:
                diff_found = True
                diffs = list(differ.compare([line for _, line in decay_belle2[particle]][change[1]:change[2]],
                             [line for _, line in special_files[particle]][change[3]:change[4]]))

                diffs_formatted = ['\n'.join([d.strip('\n') for d in diffs[n:n+4]]) for n in range(0, len(diffs), 4)]

                linenumbers_decfile = [lineno for lineno, _ in decay_belle2[particle]][change[1]:change[2]]
                linenumbers_specialdecfile = [lineno for lineno, _ in special_files[particle]][change[3]:change[4]]
                if '+' in particle or '-' in particle:
                    prefix = 'CHARGEDRARE_BELLE2.DEC'
                else:
                    prefix = 'MIXEDRARE_BELLE2.DEC'

                sys.stderr.writelines(f'Line DECAY_BELLE2.DEC: {ld} \nLine {prefix}: '
                                      f'{lspecial} \n{d}\n\n' for ld, lspecial, d in
                                      (zip(linenumbers_decfile, linenumbers_specialdecfile, diffs_formatted)))

if diff_found:
    raise RuntimeError("File differ, check above which lines don't match.")
