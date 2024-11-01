include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(
  test_mosek
  test_mosek.cc
)
target_link_libraries(test_mosek PRIVATE MOSEK::MOSEK GTest::gtest_main)
target_include_directories(test_mosek PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
gtest_discover_tests(test_mosek)
