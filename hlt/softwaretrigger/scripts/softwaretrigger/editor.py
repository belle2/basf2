import curses
import curses.textpad
from time import sleep

from softwaretrigger.db_access import upload_cut_to_db, download_cut_from_db
from softwaretrigger.display import show_menu, show_text_box


def _download_cut(window, base_name, cut_name):
    window.clear()

    window.addstr(3, 10, "You have asked for cut {base_name}/{cut_name}".format(base_name=base_name, cut_name=cut_name))
    window.addstr(5, 12, "Now downloading the cut... ")
    window.refresh()

    # download
    software_trigger_cut = download_cut_from_db(base_name, cut_name)

    window.addstr("done.")
    window.refresh()
    sleep(0.1)

    return software_trigger_cut


def _ask_for_cut(window):
    base_name = show_text_box(window, "Fill in base name")
    if not base_name:
        return

    cut_name = show_text_box(window, "Fill in cut name")
    if not cut_name:
        return

    software_trigger_cut = _download_cut(window, base_name, cut_name)

    return software_trigger_cut


def _show_editor(window, software_trigger_cut=None):
    # FIXME: Add also two name boxes

    current_selection = "cut"  # can be cut(_editor), prescale(_editor), save or abort

    def confirm_for_abort(window):
        choice = show_menu(window, ["Leave the editor (all changes are lost)", "Continue editing"], abort_possible=True)

        return choice == 0

    def show_cut_info(window, cut):
        raise NotImplementedError

    while True:
        window.clear()
        show_cut_info(window, software_trigger_cut)
        window.refresh()
        pressed_key = window.getch()

        if pressed_key == curses.KEY_ENTER or pressed_key == ord('\n'):
            if current_selection == "cut":
                current_selection = "cut_editor"
            elif current_selection == "prescale":
                current_selection = "prescale_editor"
            elif current_selection == "save":
                upload_cut_to_db(software_trigger_cut)
                return
            elif current_selection == "abort":
                if confirm_for_abort(window):
                    return

        elif pressed_key == ord('q'):
            if confirm_for_abort(window):
                return

        elif pressed_key == curses.KEY_LEFT:
            if current_selection == "prescale":
                current_selection = "cut"
            elif current_selection == "save":
                current_selection = "abort"

        elif pressed_key == curses.KEY_RIGHT:
            if current_selection == "cut":
                current_selection = "prescale"
            elif current_selection == "abort":
                current_selection = "save"

        elif pressed_key == curses.KEY_DOWN:
            if current_selection == "cut":
                current_selection = "abort"
            elif current_selection == "prescale":
                current_selection = "save"

        elif pressed_key == curses.KEY_UP:
            if current_selection == "abort":
                current_selection = "cut"
            elif current_selection == "save":
                current_selection = "prescale"


def _main_loop(window):
    choice = show_menu(window, ["Edit an existing cut", "Create a new cut"],
                       abort_possible=True)

    if choice == 0:
        # Show the editor with preloading the software cut with the asked name
        software_trigger_cut = _ask_for_cut(window)
        if software_trigger_cut:
            _show_editor(window, software_trigger_cut)

        # User may want to go on editing
        return True

    elif choice == 1:
        # Show the editor without any preloaded software cut
        _show_editor(window)

        # User may want to go on editing
        return True

    else:
        return False


def _start_editor_in_curses(stdscr):
    curses.curs_set(False)
    stdscr.clear()

    while True:
        do_continue = _main_loop(stdscr)

        if not do_continue:
            break


def start_editor():
    curses.wrapper(_start_editor_in_curses)


if __name__ == '__main__':
    start_editor()
