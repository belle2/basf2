def createThicknessandDensity():
    IRCDCB = 16
    IRCDCF = 16
    IPhiCDCB = 144
    IPhiCDCF = 144
    IRARICHF = 3
    IPhiARICHF = 144
    IPhiTOPF = 144
    IRECLB = 3
    IRECLF = 3
    IPhiECLB = 144
    IPhiECLF = 144
    f = open('GAPS-thickness-density.xml', 'w')
    f.write('<?xml version="1.0" encoding="UTF-8"?>' + '\n')
    f.write('<CDCGAPS desc="Gaps between CDC and ECL">' + '\n')
    f.write('  <IRCDCBack desc="segmentation in R of backward">' + bytes(IRCDCB) + '</IRCDCBack>' + '\n')
    f.write('  <IPhiCDCBack desc="segmentation in Phi of backward">' + bytes(IPhiCDCB) + '</IPhiCDCBack>' + '\n')
    f.write('  <IRCDCFor desc="segmentation in R of forward">' + bytes(IRCDCF) + '</IRCDCFor>' + '\n')
    f.write('  <IPhiCDCFor desc="segmentation in Phi of forward">' + bytes(IPhiCDCF) + '</IPhiCDCFor>' + '\n')
    f.write('  <thicknesses desc="Thickness" unit="mm">' + '\n')
    # Thickness of materials in the gap between CDC and ECL at backward
    for i in range(0, IRCDCB):
        for j in range(0, IPhiCDCB):
            f.write('18' + '\t')

#    f.write('\n')
    # Thickness of materials in the gap between CDC and Arich at forward
    for i in range(0, IRCDCF):
        for j in range(0, IPhiCDCF):
            f.write('10' + '\t')

    f.write('\n')
    f.write('  </thicknesses>' + '\n')
    f.write('  <IRARICHFor desc="segmentation in R of forward">' + bytes(IRARICHF) + '</IRARICHFor>' + '\n')
    f.write('  <IPhiARICHFor desc="segmentation in Phi of forward">' + bytes(IPhiARICHF) + '</IPhiARICHFor>' + '\n')
    f.write('  <IPhiTOPFor desc="segmentation in Phi of forward">' + bytes(IPhiTOPF) + '</IPhiTOPFor>' + '\n')
    f.write('  <IRECLBack desc="segmentation in R of backward">' + bytes(IRECLB) + '</IRECLBack>' + '\n')
    f.write('  <IPhiECLBack desc="segmentation in Phi of backward">' + bytes(IPhiECLB) + '</IPhiECLBack>' + '\n')
    f.write('  <IRECLFor desc="segmentation in R of forward">' + bytes(IRECLF) + '</IRECLFor>' + '\n')
    f.write('  <IPhiECLFor desc="segmentation in Phi of forward">' + bytes(IPhiECLF) + '</IPhiECLFor>' + '\n')
    f.write('  <density desc="Density" unit="g/cm3">' + '\n')
    # Density of materials in the gap between Arich and Top at backward
    for i in range(0, IRARICHF):
        for j in range(0, IPhiARICHF):
            f.write('1.29e-3' + '\t')
    # Density of materials in the gap between Top and ECL at backward
    for i in range(0, IPhiTOPF):
        f.write('1.29e-3' + '\t')
    # Density of materials in the gap between ECL barrel and end cap at backward
    for i in range(0, IRECLB):
        for j in range(0, IPhiECLB):
            f.write('1.29e-3' + '\t')
    # Density of materials in the gap between ECL barrel and end cap at forward
    for i in range(0, IRECLF):
        for j in range(0, IPhiECLF):
            f.write('1.29e-3' + '\t')

    f.write('\n')
    f.write('  </density>' + '\n')
    f.write('</CDCGAPS>')
    f.close()


def main():
    createThicknessandDensity()
#    creatMaterial()


main()
