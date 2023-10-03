import curses
from enum import Enum

class BaseOption(Enum):
    Back = 1
    Exit = 2

class Menu:
    def __init__(self, options):
        self.options = options
        for option in options.values():
            if isinstance(option, Menu):
                setattr(option, "parent", self)
        self.selected_option = 0

    def run(self):
        curses.wrapper(self._run)

    def _run(self, stdscr):
        self.stdscr = stdscr
        stdscr.clear()
        curses.curs_set(0)
        stdscr.refresh()

        while True:
            stdscr.clear()

            for i, (option_text, _) in enumerate(self.options.items()):
                if i == self.selected_option:
                    stdscr.addstr(i + 1, 1, option_text, curses.A_REVERSE)
                else:
                    stdscr.addstr(i + 1, 1, option_text)

            key = stdscr.getch()

            if key == curses.KEY_UP:
                self.selected_option = max(0, self.selected_option - 1)
            elif key == curses.KEY_DOWN:
                self.selected_option = min(len(self.options) - 1, self.selected_option + 1)
            elif key == 10:  # Enter键
                option_text, option_value = list(self.options.items())[self.selected_option]
                if isinstance(option_value, Menu):
                    option_value.run()
                else:
                    self.perform_function(option_value)
                stdscr.refresh()

    def perform_function(self, option):
        if option == BaseOption.Back:
            self.parent.run() if self.parent else exit()
        elif option == BaseOption.Exit:
            exit()
        else:
            option(self.stdscr)
            self.wait_for_enter()

    def wait_for_enter(self):
        self.stdscr.addstr(3, 1, "Press Enter to continue...")
        self.stdscr.refresh()
        while self.stdscr.getch() != 10:
            pass

def perform_function_1(stdscr):
    stdscr.addstr(1, 1, "Performing Function 1")
    stdscr.refresh()
    stdscr.getch()

def perform_function_2(stdscr):
    stdscr.addstr(1, 1, "Performing Function 2")
    stdscr.refresh()
    stdscr.getch()

if __name__ == "__main__":
    options = {
        "Register device": perform_function_1,
        "Reset device": perform_function_2,
        "Exit": BaseOption.Exit,
        "Sub Menu": Menu({
            "Sub Option 1": perform_function_1,
            "Sub Option 2": perform_function_2,
            "Back": BaseOption.Back,
        })
    }

    menu = Menu(options)
    menu.run()

