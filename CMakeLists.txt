cmake_minimum_required(VERSION 3.12.0)
find_package(Proof REQUIRED)
proof_project(proofservice-weighing-scale VERSION 0.19.4.3)
enable_language(C)

find_package(libusb)

proof_add_target_sources(proofservice-weighing-scale
    main.cpp
    weighingscalerestserver.cpp
    weighingscalehandler.cpp
)

proof_add_target_headers(proofservice-weighing-scale
    weighingscalerestserver.h
    weighingscalehandler.h
    proofservice-weighing-scale_global.h
)

if (NOT PROOF_CLANG_TIDY)
    proof_add_target_sources(proofservice-weighing-scale 3rdparty/hidapi/hid.c)
    proof_add_target_headers(proofservice-weighing-scale 3rdparty/hidapi/hidapi.h)
endif()

proof_add_service(proofservice-weighing-scale
    PROOF_LIBS Network
    OTHER_LIBS libusb::libusb
)
