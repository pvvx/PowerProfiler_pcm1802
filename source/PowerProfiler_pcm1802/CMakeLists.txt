cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

sdk_add_include_directories(./src)

target_sources(app PRIVATE ./src/cmd_decode.c)
target_sources(app PRIVATE ./src/cdc_acm_template.c)
target_sources(app PRIVATE ./src/i2s_drv.c)
target_sources(app PRIVATE ./src/ring_buffer.c)
target_sources(app PRIVATE ./src/usb_buffer.c)
sdk_set_main_file(./src/main.c)

project(project)
