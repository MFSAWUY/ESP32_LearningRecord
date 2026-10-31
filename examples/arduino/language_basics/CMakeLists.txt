cmake_minimum_required(VERSION 3.16)
project(language_model_tests LANGUAGES CXX)
# Optional DESKTOP tests only. ESP32 firmware is built with platformio.ini.
add_executable(model_tests src/model.cpp tests/test_model.cpp)
target_include_directories(model_tests PRIVATE include)
target_compile_features(model_tests PRIVATE cxx_std_17)
enable_testing()
add_test(NAME model_contract COMMAND model_tests)
