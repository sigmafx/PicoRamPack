add_executable(rampack
    main.cpp
    button.cpp
    )

# by default the header is generated into the build dir
pico_generate_pio_header(rampack ${CMAKE_CURRENT_LIST_DIR}/rampack.pio)

target_link_libraries(
    rampack
    pico_stdlib
    hardware_pio
    hardware_dma
    hardware_sync
    pico_multicore
    pico_cyw43_arch_none)

pico_add_extra_outputs(rampack)

pico_enable_stdio_usb(rampack 1)
pico_enable_stdio_uart(rampack 0)