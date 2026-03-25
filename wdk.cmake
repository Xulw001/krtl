set(WDK_WINVER "0x0A00" CACHE STRING "Default WINVER for WDK targets" )
set(WDK_NTDDI_VERSION "" CACHE STRING "Specified NTDDI_VERSION for WDK targets if needed")
set(WDK_SIGN_STORE "My" CACHE STRING "Specified Store to open when searching for the cert")
set(WDK_SIGN_NAME "" CACHE STRING "Specify the Subject Name of the signing cert, or a substring")

if(NOT WDK_ROOT)
    get_filename_component(WDK_ROOT
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
        ABSOLUTE)
endif()

if(NOT WDK_VERSION)
    file(GLOB WDK_NTDDK_FILES ${WDK_ROOT}/Include/*/km/ntddk.h)

    if(WDK_NTDDK_FILES)
        if (NOT CMAKE_VERSION VERSION_LESS 3.18.0)
            list(SORT WDK_NTDDK_FILES COMPARE NATURAL) # sort to use the latest available WDK
        endif()
        list(GET WDK_NTDDK_FILES -1 WDK_LATEST_NTDDK_FILE)
    endif()

    string(REGEX MATCH "([0-9]+(\\.[0-9]+)+)" WDK_VERSION "${WDK_NTDDK_FILES}")
endif()

if(NOT ARCH)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x64")
    else()
        set(ARCH "x86")
    endif()
endif()

set(WDK_INC_ROOT "${WDK_ROOT}/Include/${WDK_VERSION}")
set(WDK_LIB_ROOT "${WDK_ROOT}/Lib/${WDK_VERSION}")
set(WDK_BIN_ROOT "${WDK_ROOT}/Bin/${WDK_VERSION}")

set(WDK_INC "${WDK_INC_ROOT}/km;${WDK_INC_ROOT}/km/crt")
set(WDK_COMPILE_FLAGS 
    "/W4" # 警告等级
    "/Gy" # 启用函数级链接
    "/Zc:wchar_t-" # 将WChar_t视为内置类型
    "/guard:cf" # 控制流防护
    "/FI${WDK_INC_ROOT}/shared/warning.h" #
    "/Zp8" # 结构体8字节对齐
    "/GF" #
    "/WX" # 将警告视为错误
    "/GR-" # 禁用运行时类型消息
    "/Gz" # 调用约定__stdcall
    "/Oy-" # 
    "/GS" # 启用安全检查
    "/D WINNT=1")

set(WDK_LIB_DIR "${WDK_LIB_ROOT}/km/${ARCH}")
set(WDK_LINK_FLAGS
    "/MANIFEST:NO"
    "/Driver" 
    "/NODEFAULTLIB"
    "/PROFILE"
    "/WX"
    "/OPT:REF"
    "/VERSION:10.0"
    "/SUBSYSTEM:NATIVE"
    "/INCREMENTAL:NO"
    "/SECTION:INIT,d"
    "/OPT:ICF"
)

if(ARCH STREQUAL "x64")
    list(APPEND WDK_COMPILE_FLAGS "/D _WIN64" "/D _AMD64_" "/D AMD64")
    list(APPEND WDK_LINK_FLAGS "/ENTRY:GsDriverEntry")
else()
    list(APPEND WDK_COMPILE_FLAGS "/D _X86_=1" "/D i386=1" "/D STD_CALL")
    list(APPEND WDK_LINK_FLAGS "/ENTRY:GsDriverEntry@8")
endif()

macro(move_variable src dst)
    set(var "${${dst}}")
    set(${dst} "${${src}}")
    set(${src} "${var}")
endmacro()

macro(wdk_clean_default_options)
    move_variable(CMAKE_C_FLAGS CMAKE_C_FLAGS_OLD)
    move_variable(CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_OLD)
    
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG_OLD "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG_OLD "${CMAKE_CXX_FLAGS_DEBUG}")
    move_variable(CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_OLD)
    move_variable(CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG_OLD)
    # string(REPLACE "/O2" "/Ox" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    # string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

    move_variable(CMAKE_C_STANDARD_LIBRARIES CMAKE_C_STANDARD_LIBRARIES_OLD)
    move_variable(CMAKE_CXX_STANDARD_LIBRARIES CMAKE_CXX_STANDARD_LIBRARIES_OLD)
endmacro()

macro(wdk_restore_default_options)
    move_variable(CMAKE_C_FLAGS_OLD CMAKE_C_FLAGS)
    move_variable(CMAKE_CXX_FLAGS_OLD CMAKE_CXX_FLAGS)
    
    move_variable(CMAKE_C_FLAGS_DEBUG_OLD CMAKE_C_FLAGS_DEBUG)
    move_variable(CMAKE_CXX_FLAGS_DEBUG_OLD CMAKE_CXX_FLAGS_DEBUG)

    move_variable(CMAKE_C_STANDARD_LIBRARIES_OLD CMAKE_C_STANDARD_LIBRARIES)
    move_variable(CMAKE_CXX_STANDARD_LIBRARIES_OLD CMAKE_CXX_STANDARD_LIBRARIES)
endmacro()

function(wdk_add_library _target)
    cmake_parse_arguments(WDK "" "WINVER;NTDDI_VERSION" "" ${ARGN})
    add_library(${_target} STATIC ${WDK_UNPARSED_ARGUMENTS})

    target_include_directories(${_target} SYSTEM PRIVATE ${WDK_INC})
    target_compile_options(${_target} PRIVATE ${WDK_COMPILE_FLAGS})
    target_compile_definitions(${_target} PRIVATE -D_WIN32_WINNT=${WDK_WINVER} PRIVATE -DWINVER=${WDK_WINVER})
    if(WDK_NTDDI_VERSION)
        target_compile_definitions(${_target} PRIVATE -DNTDDI_VERSION=${WDK_NTDDI_VERSION})
    endif()
endfunction(wdk_add_library)

function(wdk_add_driver _target)
    cmake_parse_arguments(WDK "" "WINVER;NTDDI_VERSION;SIGN_STORE;SIGN_NAME" "LIBS" ${ARGN})
    add_executable(${_target} ${WDK_UNPARSED_ARGUMENTS})
    set_target_properties(${_target} PROPERTIES SUFFIX ".sys")

    target_include_directories(${_target} SYSTEM PRIVATE ${WDK_INC})
    target_compile_options(${_target} PRIVATE ${WDK_COMPILE_FLAGS})
    target_compile_definitions(${_target} PRIVATE -D_WIN32_WINNT=${WDK_WINVER} PRIVATE -DWINVER=${WDK_WINVER})
    if(WDK_NTDDI_VERSION)
        target_compile_definitions(${_target} PRIVATE -DNTDDI_VERSION=${WDK_NTDDI_VERSION})
    endif()

    set(DEFAULT_LIBS "BufferOverflowFastFailK" "ntoskrnl" "hal" "wmilib")

    foreach(WDK_LIB ${WDK_LIBS})
        if(WDK_LIB IN_LIST DEFAULT_LIBS)
            continue()
        endif()
        target_link_libraries(${_target} PRIVATE ${WDK_LIB_DIR}/${WDK_LIB}.lib)
    endforeach()

    foreach(WDK_LIB ${DEFAULT_LIBS})
        target_link_libraries(${_target} PRIVATE ${WDK_LIB_DIR}/${WDK_LIB}.lib)
    endforeach()

    target_link_options(${_target} PRIVATE ${WDK_LINK_FLAGS} PRIVATE "/MERGE:_TEXT=.text\;_PAGE=PAGE")

    if(WDK_SIGN_NAME)
        add_custom_command(
            TARGET ${_target} POST_BUILD
            COMMAND "${WDK_SIGNTOOL}" sign /fd SHA256 /s ${WDK_SIGN_STORE} /n ${WDK_SIGN_NAME} "$<TARGET_FILE:${_target}>"
            VERBATIM
        )
    else()
        add_custom_command(
            TARGET ${_target} POST_BUILD
            COMMAND "${WDK_SIGNTOOL}" sign /fd SHA256 /a "$<TARGET_FILE:${_target}>"
            VERBATIM
        )
    endif()
endfunction()

find_program(WDK_SIGNTOOL signtool
    HINTS "${WDK_BIN_ROOT}"
    PATH_SUFFIXES
        "x64"
        "x86"
    REQUIRED
)
