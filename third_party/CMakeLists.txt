include(FetchContent)
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/SDL-mirror/SDL.git
    GIT_TAG 76468465a94b59f0dde4791846bc2476be4f245f # 2.0.12
)
FetchContent_MakeAvailable(SDL2)

add_library(sdl2 INTERFACE)

target_include_directories(sdl2
    INTERFACE
        # SDL doesn't really support sub-builds like this.
        ${SDL2_SOURCE_DIR}/include
)

target_compile_definitions(sdl2
    INTERFACE
        SDL_MAIN_HANDLED
)

target_link_libraries(sdl2
    INTERFACE
        SDL2-static
)
