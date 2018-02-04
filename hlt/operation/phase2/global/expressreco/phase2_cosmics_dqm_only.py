
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_expressreco_dqm, finalize_hlt_path


setup_basf2_and_db()
path = create_hlt_path()

add_expressreco_processing(path, run_type="cosmics", do_reconstruction=False)

finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
