import matplotlib.pyplot as plt
import uproot


f = uproot.open("masterclass.root")
keys0 = f.keys()

data = []
for k in keys0:
    print(f[k]['BEvent'].keys())

    a = f[k]['BEvent']['m_nprt'].array()
    print(len(a))
    data.append(a)

plt.hist(data, histtype='step', bins=50, range=(0, 50))
plt.savefig('nprt.png')
plt.close()
