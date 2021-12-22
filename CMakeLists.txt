cmake_minimum_required(VERSION 3.16)
project(relayboard-control VERSION 0.1)
if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt5 5.11 REQUIRED CONFIG COMPONENTS Core Mqtt)

find_package(ECM 5.52.0 REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

find_package(KF5 5.54 REQUIRED Config)

find_package (PkgConfig REQUIRED)
pkg_check_modules(SYSTEMD REQUIRED systemd)
pkg_get_variable(SYSTEMD_SYSTEMUNITDIR systemd systemdsystemunitdir)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_executable(relayboard-control)
target_sources(relayboard-control
    PRIVATE
    main.cpp
    config.cpp
    inputthread.cpp
    mqttclient.cpp
)

target_link_libraries(relayboard-control
    bcm2835
    Qt5::Core
    Qt5::Mqtt
    KF5::ConfigCore
)

install(TARGETS relayboard-control ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES relayboard-control.service DESTINATION ${SYSTEMD_SYSTEMUNITDIR})
