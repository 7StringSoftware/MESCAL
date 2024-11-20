cmake_minimum_required(VERSION 3.22)

project(SIMPLE_MESH_GRADIENT VERSION 0.0.1)

 add_subdirectory(JUCE)
 add_subdirectory(mescal)

juce_add_gui_app(SimpleMeshGradient
    PRODUCT_NAME "SimpleMeshGradient")

juce_generate_juce_header(SimpleMeshGradient)

target_sources(SimpleMeshGradient
    PRIVATE
        Source/Main.cpp
        Source/SimpleMeshGradient.h)
target_compile_definitions(SimpleMeshGradient
    PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    DONT_SET_USING_JUCE_NAMESPACE=1
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:SimpleMeshGradient,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:SimpleMeshGradient,JUCE_VERSION>")

target_link_libraries(SimpleMeshGradient
    PRIVATE
    PUBLIC
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        mescal
        juce::juce_recommended_warning_flags)
