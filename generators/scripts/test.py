import basf2
import generators


path = basf2.create_path()

generators.add_cosmics_generator(path)
