# FindGLFW
# Mostly adapted from glfw's CMakeLists.txt

find_package(OpenGL)
find_package(Threads)

if (WIN32)
    set(_GLFW_WIN32 1)
    set(_GLFW_WGL 1)
elseif (APPLE)
    set(_GLFW_COCOA 1)
    set(_GLFW_NSGL 1)
elseif (UNIX)
    set(_GLFW_X11 1)
    set(_GLFW_GLX 1)
else()
    message(WARNING "No supported platform was detected")
endif()

#--------------------------------------------------------------------
# Use Win32 for window creation
#--------------------------------------------------------------------
if (_GLFW_WIN32)
    list(APPEND GLFW3_LIBRARIES winmm)

    # HACK: When building on MinGW, WINVER and UNICODE need to be defined before
    # the inclusion of stddef.h (by glfw3.h), which is itself included before
    # win32_platform.h.  We define them here until a saner solution can be found
    # NOTE: MinGW-w64 and Visual C++ do /not/ need this hack.
    # add_definitions(-DUNICODE)
    # add_definitions(-DWINVER=0x0501)
endif()

#--------------------------------------------------------------------
# Use WGL for context creation
#--------------------------------------------------------------------
if (_GLFW_WGL)
    list(APPEND GLFW3_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND GLFW3_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()

#--------------------------------------------------------------------
# Use X11 for window creation
#--------------------------------------------------------------------
if (_GLFW_X11)

    find_package(X11 REQUIRED)

    # Set up library and include paths
    list(APPEND GLFW3_INCLUDE_DIRS ${X11_X11_INCLUDE_PATH})
    list(APPEND GLFW3_LIBRARIES ${X11_X11_LIB} ${CMAKE_THREAD_LIBS_INIT})
    if (UNIX AND NOT APPLE)
        list(APPEND GLFW3_LIBRARIES ${RT_LIBRARY})
    endif()

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_FOUND)
        message(WARNING "The RandR library and headers were not found")
    endif()

    list(APPEND GLFW3_INCLUDE_DIRS ${X11_Xrandr_INCLUDE_PATH})
    list(APPEND GLFW3_LIBRARIES ${X11_Xrandr_LIB})

    # Check for XInput (high-resolution cursor motion)
    if (NOT X11_Xinput_FOUND)
        message(WARNING "The XInput library and headers were not found")
    endif()

    list(APPEND GLFW3_INCLUDE_DIRS ${X11_Xinput_INCLUDE_PATH})

    if (X11_Xinput_LIB)
        list(APPEND GLFW3_LIBRARIES ${X11_Xinput_LIB})
    else()
        # Backwards compatibility (bug in CMake 2.8.7)
        list(APPEND GLFW3_LIBRARIES Xi)
    endif()

    # Check for Xf86VidMode (fallback gamma control)
    if (NOT X11_xf86vmode_FOUND)
        message(WARNING "The Xf86VidMode library and headers were not found")
    endif()

    list(APPEND GLFW3_INCLUDE_DIRS ${X11_xf86vmode_INCLUDE_PATH})

    if (X11_Xxf86vm_LIB)
        list(APPEND GLFW3_LIBRARIES ${X11_Xxf86vm_LIB})
    else()
        # Backwards compatibility (see CMake bug 0006976)
        list(APPEND GLFW3_LIBRARIES Xxf86vm)
    endif()

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_FOUND)
        message(WARNING "The X keyboard extension headers were not found")
    endif()

    list(APPEND glfw_INCLUDE_DIR ${X11_Xkb_INCLUDE_PATH})

    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        list(APPEND GLFW3_LIBRARIES ${RT_LIBRARY})
    endif()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        list(APPEND GLFW3_LIBRARIES ${MATH_LIBRARY})
    endif()

endif()

#--------------------------------------------------------------------
# Use GLX for context creation
#--------------------------------------------------------------------
if (_GLFW_GLX)
    list(APPEND GLFW3_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    list(APPEND GLFW3_LIBRARIES ${OPENGL_gl_LIBRARY})

    list(APPEND glfw_LIBRARIES ${CMAKE_DL_LIBS})

endif()

#--------------------------------------------------------------------
# Use EGL for context creation
#--------------------------------------------------------------------
#if (_GLFW_EGL)
#
#    list(APPEND GLFW3_INCLUDE_DIRS ${EGL_INCLUDE_DIR})
#    list(APPEND GLFW3_LIBRARIES ${EGL_LIBRARY})
#
#    if (UNIX)
#        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} egl")
#    endif()
#
#    if (_GLFW_USE_OPENGL)
#        list(APPEND GLFW3_LIBRARIES ${OPENGL_gl_LIBRARY})
#        list(APPEND GLFW3_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
#        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} gl")
#    elseif (_GLFW_USE_GLESV1)
#        list(APPEND GLFW3_LIBRARIES ${GLESv1_LIBRARY})
#        list(APPEND GLFW3_INCLUDE_DIRS ${GLESv1_INCLUDE_DIR})
#        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} glesv1_cm")
#    elseif (_GLFW_USE_GLESV2)
#        list(APPEND GLFW3_LIBRARIES ${GLESv2_LIBRARY})
#        list(APPEND GLFW3_INCLUDE_DIRS ${GLESv2_INCLUDE_DIR})
#        set(GLFW_PKG_DEPS "${GLFW_PKG_DEPS} glesv2")
#    endif()
#
#endif()

#--------------------------------------------------------------------
# Use Cocoa for window creation and NSOpenGL for context creation
#--------------------------------------------------------------------
if (_GLFW_COCOA AND _GLFW_NSGL)

    # Set up library and include paths
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)
    list(APPEND GLFW3_LIBRARIES ${COCOA_FRAMEWORK}
            ${OPENGL_gl_LIBRARY}
            ${IOKIT_FRAMEWORK}
            ${CORE_FOUNDATION_FRAMEWORK}
            ${CORE_VIDEO_FRAMEWORK})

    set(GLFW3_PKG_DEPS "")
    set(GLFW3_PKG_LIBS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework CoreVideo")
endif()


find_path(GLFW3_INCLUDE_DIR GLFW/glfw3.h)
find_library(GLFW3_LIBRARY glfw3)

set(GLFW3_INCLUDE_DIRS "${GLFW3_INCLUDE_DIR};${GLFW3_INCLUDE_DIRS}")
set(GLFW3_LIBRARIES "${GLFW3_LIBRARY};${GLFW3_LIBRARIES}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW3 DEFAULT_MSG
        GLFW3_INCLUDE_DIR GLFW3_LIBRARY GLFW3_INCLUDE_DIRS GLFW3_LIBRARIES)