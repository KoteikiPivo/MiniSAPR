cmake_minimum_required(VERSION 3.16)
project(mini_sapr LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_subdirectory(src/app)
