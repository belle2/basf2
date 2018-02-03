
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_dqm, finalize_hlt_path


setup_basf2_and_db()
path = create_hlt_path()

# no reconstruction or software trigger added at all
# just add dqm
add_hlt_dqm(path, run_type="cosmics")


finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
