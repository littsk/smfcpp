import curses
from enum import Enum
import subprocess
import os

from . import uart_interface

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

def main():
    # TODO don't use hard code
    uart_path = "/dev/ttyUSB0"
    if os.path.exists(uart_path) :
        try:
            # 使用 subprocess.run 执行 chmod 命令
            subprocess.run(["sudo", "chmod", "o+rw", uart_path], check=True)
            print(f"Added read and write permission for other users to {uart_path}.")
        except Exception as e:
            exit()
    else:
        print("请连接串口设备")
        exit()
        

    options = {
        "Uart": Menu({
            "register_device": uart_interface.register_device,
            "remove_device": uart_interface.remove_device,
            "reset_device": uart_interface.reset_device,
            "list_all_devices": uart_interface.list_all_devices,
            "clear_all_devices": uart_interface.clear_all_devices,
            "Back": BaseOption.Back,
        }),
        "Exit": BaseOption.Exit,
    }

    menu = Menu(options)
    menu.run()

if __name__ == "__main__":
    main()

