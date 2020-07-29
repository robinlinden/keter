cmake_minimum_required(VERSION 3.15)

project(project_keter LANGUAGES C)

include(FetchContent)
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/SDL-mirror/SDL.git
    GIT_TAG 76468465a94b59f0dde4791846bc2476be4f245f # 2.0.12
)
FetchContent_MakeAvailable(SDL2)

add_executable(${PROJECT_NAME}
    main.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        # SDL doesn't really support sub-builds like this.
        ${SDL2_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # SDL does weird things w/ main and it doesn't work well under mingw.
        $<$<BOOL:${MINGW}>:mingw32>
        SDL2main
        SDL2-static
)
