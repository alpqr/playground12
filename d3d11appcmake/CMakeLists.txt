cmake_minimum_required(VERSION 3.1)

project(winapp VERSION 1.0 DESCRIPTION "winapp" LANGUAGES CXX)

add_compile_options("/std:c++latest")

add_executable(winapp WIN32
    main.cpp
    render.cpp
    render.h
    app.h
    event.h
    frame.cpp
    frame.h
    keymap.h
)

target_link_libraries(winapp
    d3d11.lib
    dxgi.lib
    dxguid.lib
)
