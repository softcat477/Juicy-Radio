cmake_minimum_required(VERSION 3.16)
project(Radio VERSION 0.0.1)
set (CMAKE_CXX_STANDARD 17)

#set(CMAKE_PREFIX_PATH "/Users/softcat/Softcat/JUCE/install")
set(CMAKE_CXX_FLAGS "-Wno-unused-parameter")

set (TargetName ${PROJECT_NAME}) # New var : TargetName

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

# JUCE
find_package(JUCE CONFIG REQUIRED)
juce_add_gui_app(${TargetName} PRODUCT_NAME "BBC Radio3 Radio")
target_sources(${TargetName} PRIVATE
        src/MainWindow.cpp
        src/MainComponent.cpp
        src/CondVar.cpp
        src/IThread.cpp
        src/StereoOut.cpp
        src/ChannelGui.cpp
        src/Mp3Decoder.cpp
        src/RadioReceiver.cpp
        src/Osc.cpp
        main.cpp)
target_compile_definitions(${TargetName} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>")
target_link_libraries(${TargetName} PRIVATE
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_gui_basics)

# CURL
find_package (CURL)
if (CURL_FOUND)
    message ("Include CURL from " ${CURL_INCLUDE_DIRS})
    message ("Include CURL lib " ${CURL_LIBRARIES})
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries (${TargetName} PRIVATE ${CURL_LIBRARIES})
endif (CURL_FOUND)

# libmad
find_path(LIBMAD_INCLUDE_DIRS mad.h)
find_library(LIBMAD_LIBRARIES mad)

message ("Inlucde mad.g from path: " ${LIBMAD_INCLUDE_DIRS})
message ("Include libmad from path: " ${LIBMAD_LIBRARIES})

target_link_libraries(${TargetName} PRIVATE ${LIBMAD_LIBRARIES})
target_include_directories(${TargetName} PUBLIC ${LIBMAD_INCLUDE_DIRS})

# Make libraries from Ring Buffer
#add_library(ringBuffer src/CondVar.cpp include/RingBuffer.h include/CondVar.h)
#target_include_directories(ringBuffer PUBLIC include)

# Make libraries from Wire
add_library(wire src/CondVar.cpp include/RingBuffer.h include/CondVar.h include/Wire.h include/Aggregator.h include/Distributor.h)
target_include_directories(wire PUBLIC include)


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the test sub-directory also.
add_subdirectory(tests)
