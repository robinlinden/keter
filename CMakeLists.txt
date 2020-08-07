cmake_minimum_required(VERSION 3.15)

project(project_keter LANGUAGES C)

add_subdirectory(third_party)

add_executable(${PROJECT_NAME}
    main.c
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # SDL does weird things w/ main and it doesn't work well under mingw.
        $<$<BOOL:${MINGW}>:mingw32>
        sdl2
)

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -pedantic-errors;
        -Wall;
        -Werror;
        -Wextra;
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /permissive-;
        /W4;
        /WX;
    >
)
