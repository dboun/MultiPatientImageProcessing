project(Test${MODULE_NAME})

enable_testing()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

add_executable(test test.cpp)
add_test(test test)

target_link_libraries( test 
    ${MODULE_NAME}    
    Qt5::Test
)

add_custom_command(
    TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_LIST_DIR}/test_image.nii.gz
            ${CMAKE_CURRENT_BINARY_DIR}/test_image.nii.gz
)

add_custom_command(
    TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_LIST_DIR}/test_image.nrrd
            ${CMAKE_CURRENT_BINARY_DIR}/test_image.nrrd
)