include(ExternalProject)
ExternalProject_Add(Kvasir EXCLUDE_FROM_ALL 1
        URL https://github.com/kvasir-io/mpl/archive/development.zip
        TIMEOUT 120
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/dependencies/kvasir"
        CONFIGURE_COMMAND "" # Disable configure step
        BUILD_COMMAND ""     # Disable build step
        INSTALL_COMMAND ""   # Disable install step
        TEST_COMMAND ""      # Disable test step
        UPDATE_COMMAND ""    # Disable source work-tree update
        )
ExternalProject_Get_Property(Kvasir SOURCE_DIR)
set(Kvasir_INCLUDE_DIR ${SOURCE_DIR}/src)

target_include_directories(dimeta INTERFACE ${Kvasir_INCLUDE_DIR})
add_dependencies(dimeta Kvasir)