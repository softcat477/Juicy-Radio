#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../include/MainComponent.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <string>

namespace AudioApp
{
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name);

private:
    void closeButtonPressed() override;
    juce::Colour getBackgroundColour();
    MainComponent* _main_component;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
}

#endif