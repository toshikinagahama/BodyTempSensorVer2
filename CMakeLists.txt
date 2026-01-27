cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(BodyTempSensorVer2)

file(GLOB_RECURSE APP_SOURCES src/*.c)

target_sources(app PRIVATE ${APP_SOURCES})

# drivers 以下のサブディレクトリを自動でインクルードパスに追加する
file(GLOB_RECURSE driver_headers "src/drivers/*.h")
foreach(header_file ${driver_headers})
    get_filename_component(header_dir ${header_file} DIRECTORY)
    target_include_directories(app PRIVATE ${header_dir})
endforeach()

# utils 以下のサブディレクトリを自動でインクルードパスに追加する
file(GLOB_RECURSE utils_headers "src/utils/*.h")
foreach(header_file ${utils_headers})
    get_filename_component(header_dir ${header_file} DIRECTORY)
    target_include_directories(app PRIVATE ${header_dir})
endforeach()
