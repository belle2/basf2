
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, add_hlt_dqm, finalize_hlt_path


setup_basf2_and_db()
path = create_hlt_path()

# no reconstruction or software trigger added at all

add_hlt_dqm(path, run_type="collisions")

finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
