def creatMaterial():
    f = open('ECLGAPS-Materials.xml', 'w')
    f.write('<?xml version="1.0" encoding="UTF-8"?>' + '\n')
    f.write('<Materials>' + '\n')
# Materials of the gap between Arich and Top at backward
    for i in range(0, 3):
        for j in range(0, 144):
            f.write('  <Material name="ArichGap_' + bytes(i) + '_' + bytes(j) + '">' + '\n')
            f.write('    <density unit="g/cm3">1.29e-3</density>' + '\n')
            f.write('    <Components>' + '\n')
            f.write('      <Element fraction="0.7">N</Element>' + '\n')
            f.write('      <Element fraction="0.3">O</Element>' + '\n')
            f.write('    </Components>' + '\n')
            f.write('  </Material>' + '\n')
            f.write('\n')

# Materials of the gap between Top and ECL at backward
    for i in range(0, 144):
        f.write('  <Material name="TopGap_' + bytes(i) + '">' + '\n')
        f.write('    <density unit="g/cm3">1.29e-3</density>' + '\n')
        f.write('    <Components>' + '\n')
        f.write('      <Element fraction="0.7">N</Element>' + '\n')
        f.write('      <Element fraction="0.3">O</Element>' + '\n')
        f.write('    </Components>' + '\n')
        f.write('  </Material>' + '\n')
        f.write('\n')

# Materials of the gap between ECL barrel and end cap at backward
    for i in range(0, 3):
        for j in range(0, 144):
            f.write('  <Material name="ECLback_' + bytes(i) + '_' + bytes(j) + '">' + '\n')
            f.write('    <density unit="g/cm3">1.29e-3</density>' + '\n')
            f.write('    <Components>' + '\n')
            f.write('      <Element fraction="0.7">N</Element>' + '\n')
            f.write('      <Element fraction="0.3">O</Element>' + '\n')
            f.write('    </Components>' + '\n')
            f.write('  </Material>' + '\n')
            f.write('\n')

# Materials of the gap between ECL barrel and end cap at forward
    for i in range(0, 3):
        for j in range(0, 144):
            f.write('  <Material name="ECLfor_' + bytes(i) + '_' + bytes(j) + '">' + '\n')
            f.write('    <density unit="g/cm3">1.29e-3</density>' + '\n')
            f.write('    <Components>' + '\n')
            f.write('      <Element fraction="0.7">N</Element>' + '\n')
            f.write('      <Element fraction="0.3">O</Element>' + '\n')
            f.write('    </Components>' + '\n')
            f.write('  </Material>' + '\n')
# f.write('\n')
    f.write('</Materials>' + '\n')
    f.close()


def createthickness():
    f = open('CDCGAPS-thickness.xml', 'w')
    f.write('<?xml version="1.0" encoding="UTF-8"?>' + '\n')
    f.write('<CDCGAPS desc="Gaps between CDC and ECL">' + '\n')
    f.write('  <BackwardGAP desc="Backward Gap">' + '\n')
    for i in range(0, 16):
        for j in range(0, 144):
            f.write(
                '    <Bthick_' +
                bytes(i) +
                '_' +
                bytes(j) +
                ' desc="thickness" unit="mm">18</Bthick_' +
                bytes(i) +
                '_' +
                bytes(j) +
                '>' +
                '\n')
    f.write('  </BackwardGAP>' + '\n')
    f.write('  <ForwardGAP desc="Backward Gap">' + '\n')
    for i in range(0, 16):
        for j in range(0, 144):
            f.write(
                '    <Fthick_' +
                bytes(i) +
                '_' +
                bytes(j) +
                ' desc="thickness" unit="mm">10</Fthick_' +
                bytes(i) +
                '_' +
                bytes(j) +
                '>' +
                '\n')
    f.write('  </ForwardGAP>' + '\n')
    f.write('</CDCGAPS>')
    f.close()


def main():
    # createthickness()
    creatMaterial()


main()
