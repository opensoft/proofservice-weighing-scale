find_path(libusb_INCLUDE_DIRS NAMES libusb.h
    HINTS $ENV{LIBUSB_DIR}
    PATH_SUFFIXES include/libusb-1.0 include
    PATHS ~/Library/Frameworks /Library/Frameworks /usr/local /usr
    DOC "libusb-1.0 include directory"
)

find_library(libusb_LIBRARIES
    NAMES usb-1.0 usb
    HINTS $ENV{LIBUSB_DIR}
    PATH_SUFFIXES x86_64-linux-gnu
    PATHS /usr /usr/local
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libusb REQUIRED_VARS libusb_INCLUDE_DIRS libusb_LIBRARIES)

if(NOT libusb_FOUND)
    return()
endif()

if(NOT TARGET libusb::libusb)
    add_library(libusb::libusb INTERFACE IMPORTED)
endif()
set_property(TARGET libusb::libusb PROPERTY INTERFACE_LINK_LIBRARIES "${libusb_LIBRARIES}" pthread)
set_property(TARGET libusb::libusb PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${libusb_INCLUDE_DIRS}")
