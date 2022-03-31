cmake_minimum_required(VERSION 3.16)
project(Radio VERSION 0.0.1)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_PREFIX_PATH "/Users/softcat/Softcat/JUCE/install")


#FILE(GLOB MyCSources /Users/softcat/Softcat/lame-svn-r6507-trunk-lame/frontend/*.c)
#add_executable(Radio main.cpp ${MyCSources})
#add_executable(Radio oldMain.cpp)
#dd_executable(Radio main.cpp)
set (TargetName ${PROJECT_NAME}) # New var : TargetName

# JUCE
find_package(JUCE CONFIG REQUIRED)
juce_add_gui_app(${TargetName} PRODUCT_NAME "BBC Radio3 Radio")
target_sources(${TargetName} PRIVATE
        src/MainWindow.cpp
        src/MainComponent.cpp
        src/ThreadInternet.cpp
        src/ThreadDecoder.cpp
        src/CondVar.cpp
        src/IThreadManager.cpp
        src/ThreadChannel.cpp
        src/ChannelStrip.cpp
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


#target_link_libraries(Radio -lmp3lame)
#target_link_libraries(Radio /usr/local/lib/libmp3lame.dylib)

# >>> Print linked libraries
#get_target_property(OUT Radio LINK_LIBRARIES)
#message ("Linked libraries: " ${OUT})

#target_include_directories(Radio PUBLIC /Users/softcat/Softcat/lame-svn-r6507-trunk-lame/include)
#target_include_directories(Radio PUBLIC /Users/softcat/Softcat/lame-svn-r6507-trunk-lame/frontend)
#target_include_directories(Radio PUBLIC /Users/softcat/Softcat/lame-svn-r6507-trunk-lame/libmp3lame)
#message(STATUS ${OUT})


#set (BaseTargetName Radio)

#juce_add_plugin("${BaseTargetName}"
#        # VERSION ...                               # Set this if the plugin version is different to the project version
#        # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
#        # ICON_SMALL ...
#        COMPANY_NAME "MyCompany"
#        IS_SYNTH FALSE
#        NEEDS_MIDI_INPUT TRUE
#        NEEDS_MIDI_OUTPUT FALSE
#        IS_MIDI_EFFECT FALSE
#        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
#        COPY_PLUGIN_AFTER_BUILD TRUE
#        PLUGIN_MANUFACTURER_CODE Juce
#        PLUGIN_CODE Dem0
#        FORMATS AU VST3 Standalone
#        PRODUCT_NAME "Audio Plugin Example")

#target_sources(${BaseTargetName} PRIVATE
#        Source/Voice.cpp
#        source/AudioEngine.cpp
#        Source/DSPTutorialAudioProcessor.cpp
#        Source/PluginEditor.cpp)

#target_compile_definitions(${BaseTargetName}
#        PUBLIC
#        JUCE_WEB_BROWSER=0
#        JUCE_USE_CURL=0
#        JUCE_VST3_CAN_REPLACE_VST2=0)

#target_link_libraries(${BaseTargetName} PRIVATE
#        juce::juce_audio_utils
#        juce::juce_dsp
#        juce::juce_recommended_config_flags
#        juce::juce_recommended_lto_flags
#        juce::juce_recommended_warning_flags)
