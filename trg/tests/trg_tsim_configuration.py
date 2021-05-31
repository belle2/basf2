import basf2 as b2
import L1trigger as tsim

main = b2.Path()

tsim.add_trigger_simulation(path=main, PrintInfo=True)
