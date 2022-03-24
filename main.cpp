/*
#include <iostream>
#include <stdlib.h>

#include "include/RingBuffer.h"
#include "include/ThreadInternet.h"

int main(){
    int sample_per_frame = 1400;
    int max_frame_count = 10;

    RingBuffer<char> buf{sample_per_frame, max_frame_count};
    ThreadInternet thread_internet{&buf};

    printf("Hello world!\n");

    return 0;
}
*/

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include "include/MainWindow.h"

namespace AudioApp
{
class GuiAppTemplateApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& /*commandLine*/) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override { mainWindow.reset(); }

    void systemRequestedQuit() override { quit(); }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {}

private:
    std::unique_ptr<MainWindow> mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(GuiAppTemplateApplication)

} // namespace GuiApp
