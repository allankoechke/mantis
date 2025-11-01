add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/bcrypt-cpp)
target_link_libraries ( mantis PUBLIC bcrypt_cpp )