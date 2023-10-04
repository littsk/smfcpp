import curses
import uart

def get_device_type(stdscr):
    device_types = [uart.SensorDeviceType.SoilSensor, uart.SensorDeviceType.AirSensor]
    current_type = 0

    while True:
        stdscr.clear()
        stdscr.addstr(0, 0, "Register Device")
        stdscr.addstr(2, 0, "Select device type (use arrow keys, Enter to confirm):")
        
        # 绘制设备类型列表
        for i, device_type in enumerate(device_types):
            if i == current_type:
                stdscr.attron(curses.A_REVERSE)
            stdscr.addstr(i + 3, 0, f"{i + 1}. {device_type}")
            stdscr.attroff(curses.A_REVERSE)

        stdscr.addstr(6, 0, f"Selected Device Type: {device_types[current_type]}")
        stdscr.refresh()

        key = stdscr.getch()

        if key == curses.KEY_UP:
            current_type = (current_type - 1) % len(device_types)
        elif key == curses.KEY_DOWN:
            current_type = (current_type + 1) % len(device_types)
        elif key == 10:  # Enter key
            return device_types[current_type]

def register_device(stdscr):
    stdscr.clear()
    stdscr.addstr(0, 0, "Register Device")
    stdscr.addstr(2, 0, "Enter device ID: ")
    
    input_string = ""  # 用于存储用户输入的字符

    while True:
        stdscr.addstr(4, 0, input_string)
        stdscr.refresh()

        key = stdscr.getch()

        if key == 10:  # Enter key
            break
        elif key == 127:  # Backspace key
            input_string = input_string[:-1]
        elif 32 <= key <= 126:  # ASCII printable characters
            input_string += chr(key)

    device_type = get_device_type(stdscr)
    device_id = input_string  # 使用输入的字符作为设备ID

    if uart.register_device(device_type, int(device_id)):
        stdscr.addstr(6, 0, "Device registered successfully. Press any key to continue.")
    else:
        stdscr.addstr(6, 0, "Device registration failed. Press any key to continue.")
    stdscr.refresh()
    stdscr.getch()

def list_devices(stdscr):
    stdscr.clear()
    stdscr.addstr(0, 0, "List of Devices")
    devices = uart.list_all_devices()
    for i, (device_id, device_type) in enumerate(devices):
        stdscr.addstr(i + 2, 0, f"Device ID: {device_id}, Device Type: {device_type}")
    stdscr.addstr(len(devices) + 3, 0, "Press any key to continue.")
    stdscr.refresh()
    stdscr.getch()

def remove_device(stdscr):
    stdscr.clear()
    stdscr.addstr(0, 0, "Remove Device")
    stdscr.addstr(2, 0, "Enter device ID to remove: ")

    input_string = ""  # 用于存储用户输入的字符

    while True:
        stdscr.addstr(4, 0, input_string)
        stdscr.refresh()

        key = stdscr.getch()

        if key == 10:  # Enter key
            break
        elif key == 127:  # Backspace key
            input_string = input_string[:-1]
        elif 32 <= key <= 126:  # ASCII printable characters
            input_string += chr(key)

    device_id = input_string  # 使用输入的字符作为设备ID
    if uart.remove_device(int(device_id)):
        stdscr.addstr(4, 0, "Device removed successfully. Press any key to continue.")
    else:
        stdscr.addstr(4, 0, "Device removal failed. Press any key to continue.")
    stdscr.refresh()
    stdscr.getch()

def clear_all_devices(stdscr):
    stdscr.clear()
    stdscr.addstr(0, 0, "Clear All Devices")
    stdscr.addstr(2, 0, "Are you sure you want to clear all devices? (Y/N): ")
    stdscr.refresh()

    input_string = ""  # 用于存储用户输入的字符

    while True:
        stdscr.addstr(4, 0, input_string)
        stdscr.refresh()

        key = stdscr.getch()

        if key == 10:  # Enter key
            break
        elif key == 127:  # Backspace key
            input_string = input_string[:-1]
        elif 32 <= key <= 126:  # ASCII printable characters
            input_string += chr(key)

    confirm = input_string

    if confirm == ord('Y') or confirm == ord('y'):
        if uart.clear_all_devices():
            stdscr.addstr(4, 0, "All devices cleared successfully. Press any key to continue.")
        else:
            stdscr.addstr(4, 0, "Device clearing failed. Press any key to continue.")
    else:
        stdscr.addstr(4, 0, "Clearing canceled. Press any key to continue.")
    stdscr.refresh()
    stdscr.getch()

def main(stdscr):
    curses.curs_set(0)
    curses.nodelay(1)
    stdscr.clear()
    stdscr.refresh()

    current_option = 0
    options = ["Register Device", "List Devices", "Remove Device", "Clear All Devices"]

    while True:
        stdscr.clear()
        stdscr.addstr(0, 0, "UART Device Management")
        
        # 绘制选项列表
        for i, option in enumerate(options):
            if i == current_option:
                stdscr.attron(curses.A_REVERSE)
            stdscr.addstr(i + 2, 0, f"{i + 1}. {option}")
            stdscr.attroff(curses.A_REVERSE)
        
        stdscr.refresh()

        key = stdscr.getch()

        if key == curses.KEY_UP:
            current_option = (current_option - 1) % len(options)
        elif key == curses.KEY_DOWN:
            current_option = (current_option + 1) % len(options)
        elif key == 10:  # Enter key
            if current_option == 0:
                register_device(stdscr)
            elif current_option == 1:
                list_devices(stdscr)
            elif current_option == 2:
                remove_device(stdscr)
            elif current_option == 3:
                clear_all_devices(stdscr)
        elif key == 27:  # ESC key to exit
            break

if __name__ == "__main__":
    curses.wrapper(main)
