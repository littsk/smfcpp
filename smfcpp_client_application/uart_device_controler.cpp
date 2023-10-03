#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <crc.hpp>

#include "config.h"

static uint8_t addr1to2[8]    = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00}; //给设备更换地址使用
CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);

bool get_dev_cache(YAML::Node & dev_cache){
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path dev_cache_path = cache_dir / "dev_cache.yaml";

    if (!std::filesystem::exists(cache_dir)) {
        std::cout << "Cache directory didn't exist, creating it now..." << std::endl;
        std::filesystem::create_directories(cache_dir);
    }

    if (std::filesystem::exists(cache_dir)) {
        std::cout << "Cache directory create success" << std::endl;
    } else {
        std::cerr << "Unknown cache directory creation error" << std::endl;
        return false;
    }

    while (true) {
        try {
            dev_cache = YAML::LoadFile(dev_cache_path);
            break;
        } catch (YAML::BadFile const &e) {
            std::cout << "Cache file didn't exist, creating it now..." << std::endl;
            std::ofstream newfile(dev_cache_path);
            if (newfile.is_open()) {
                // 文件会在newfile作用域结束时自动关闭
                break;
            } else {
                std::cerr << "Unknown cache file creation error" << std::endl;
                return false;
            }
        } catch (YAML::Exception const &e) {
            std::cerr << "Failed to load YAML: " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}

bool save_dev_cache(YAML::Node &dev_cache) {
    std::filesystem::path cache_dir(CACHE_DIR);
    std::filesystem::path dev_cache_path = cache_dir / "dev_cache.yaml";

    YAML::Emitter emitter;
    emitter << dev_cache;

    // 打开文件以进行写入
    std::ofstream fout(dev_cache_path);
    if (!fout.is_open()) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return false;
    }

    // 写入数据到文件
    fout << emitter.c_str();

    // 检查写入是否成功
    if (fout.fail()) {
        std::cerr << "Failed to write data to the file." << std::endl;
        fout.close();
        return false;
    }

    // 关闭文件
    fout.close();
    return true;
}

class Menu {
public:
    Menu() {
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        selected_option = 0;
    }

    ~Menu() {
        endwin();
    }

    void run(const char *options[], int n_options) {
        while (1) {
            clear();

            // 显示功能列表
            for (int i = 0; i < n_options; ++i) {
                if (i == selected_option) {
                    attron(A_REVERSE);
                }
                mvprintw(i + 1, 1, options[i]);
                attroff(A_REVERSE);
            }

            // 获取用户输入
            int ch = getch();

            // 处理用户输入
            switch (ch) {
                case KEY_UP:
                    selected_option = (selected_option > 0) ? (selected_option - 1) : 0;
                    break;
                case KEY_DOWN:
                    selected_option = (selected_option < n_options - 1) ?
                                      (selected_option + 1) : (n_options - 1);
                    break;
                case 10: // Enter键
                    if (selected_option == n_options - 1) {
                        // 退出程序
                        return;
                    } else {
                        // 进入子菜单
                        runSubMenu(sub_menus[selected_option]);
                        break;
                    }
                default:
                    break;
            }
        }
    }

private:
    int selected_option;

    void performFunction(int option) {
        clear();
        // 根据选项执行相应的功能
        if (option == 0) {
            // 执行 "Register device" 功能
            mvprintw(1, 1, "Performing Register device");
        } else if (option == 1) {
            // 执行 "Reset device" 功能
            mvprintw(1, 1, "Performing Reset device");
        }

        // 在执行完功能后等待用户按回车键
        mvprintw(3, 1, "Press Enter to continue...");
        refresh();
        while (getch() != '\n') {
            // 等待用户按回车键
        }
    }

    void runSubMenu(const char *sub_menu_options[]) {
        int n_sub_options = sub_menu_options ? sizeof(sub_menu_options) / sizeof(sub_menu_options[0]) : 0;
        int sub_selected_option = 0;

        while (1) {
            clear();

            // 显示子菜单
            for (int i = 0; i < n_sub_options; ++i) {
                if (i == sub_selected_option) {
                    attron(A_REVERSE);
                }
                mvprintw(i + 1, 1, sub_menu_options[i]);
                attroff(A_REVERSE);
            }

            // 获取用户输入
            int ch = getch();

            // 处理用户输入
            switch (ch) {
                case KEY_UP:
                    sub_selected_option = (sub_selected_option > 0) ? (sub_selected_option - 1) : 0;
                    break;
                case KEY_DOWN:
                    sub_selected_option = (sub_selected_option < n_sub_options - 1) ?
                                          (sub_selected_option + 1) : (n_sub_options - 1);
                    break;
                case 10: // Enter键
                    // 根据子菜单选项执行相应的功能
                    performFunction(sub_selected_option);
                    break;
                case 27: // ESC键，返回上一级菜单
                    return;
                default:
                    break;
            }
        }
    }

    // 定义子菜单选项
    const char *sub_menu1[3] = {
        "Sub Option 1",
        "Sub Option 2",
        "Back to Main Menu"
    };

    const char *sub_menu2[3] = {
        "Sub Option A",
        "Sub Option B",
        "Back to Main Menu"
    };

    // 子菜单数组
    const char **sub_menus[2] = {
        sub_menu1,
        sub_menu2
    };
};

int main() {
    YAML::Node dev_cache;
    if (get_dev_cache(dev_cache) == false) {
        return 1;
    }

    if (!dev_cache["dev_list"]) {
        dev_cache["dev_list"] = YAML::Load("[]");
    }

    auto dev_list = dev_cache["dev_list"];

    dev_list.push_back("item1");
    dev_list.push_back("item2");
    std::cout << dev_list.size() << std::endl;

    if (save_dev_cache(dev_cache) == false) {
        return 1;
    }

    const char *options[] = {
        "Option 1",
        "Option 2",
        "Exit"
    };
    int n_options = sizeof(options) / sizeof(options[0]);

    Menu menu;
    menu.run(options, n_options);

    return 0;
}