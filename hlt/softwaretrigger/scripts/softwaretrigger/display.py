import curses


def show_menu(window, choices: list, start_x: int = 10, start_y: int = 3, abort_possible: bool = False) -> int:
    if len(choices) == 0:
        return

    current_choice = 0
    max_y, max_x = window.getmaxyx()

    def show_choices_with_selection(selected_entry):
        window.clear()
        y = start_y
        for choice in choices:
            window.addstr(y, start_x, choice)
            y += 2

        window.addch(start_y + 2 * selected_entry, start_x - 4, '>')

        if abort_possible:
            window.addstr(max_y - 2, 2, "Press q to abort.")

    while True:
        show_choices_with_selection(current_choice)

        window.refresh()
        pressed_key = window.getch()

        if pressed_key == curses.KEY_UP:
            if current_choice > 0:
                current_choice -= 1
        elif pressed_key == curses.KEY_DOWN:
            if current_choice < len(choices) - 1:
                current_choice += 1
        elif pressed_key == ord('q') or pressed_key == ord('Q'):
            if abort_possible:
                return
        elif pressed_key == curses.KEY_ENTER or pressed_key == ord('\n'):
            return current_choice


def show_text_box(window, caption):
    window.clear()

    window.addstr(3, 10, caption)
    window.addch(5, 12, '>')
    window.refresh()

    editwin = curses.newwin(1, 40, 5, 12)
    curses.textpad.rectangle(window, 5 - 1, 12 - 2, 5 - 1 + 1 + 1, 12 - 1 + 40 + 2)
    window.refresh()

    box = curses.textpad.Textbox(editwin)
    box.stripspaces = True
    curses.curs_set(True)
    box.edit()
    curses.curs_set(False)

    return box.gather().strip()
