#include "../include/MainWindow.h"
#include "../include/MainComponent.h"

namespace AudioApp
{
constexpr bool isMobile()
{
#if JUCE_IOS || JUCE_ANDROID
    return true;
#else
    return false;
#endif
}

AudioApp::MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name, getBackgroundColour(), allButtons)
{
    setUsingNativeTitleBar(true);
    //  Make the layout inside MainComponent()
    size_t sample_per_frame_radio = 8192;
    size_t max_frame_count_radio = 128;
    //size_t sample_per_frame = 1152;
    size_t sample_per_frame = 0; // Use device's default setting
    size_t max_frame_count = 512;
    _main_component = new MainComponent(sample_per_frame_radio, max_frame_count_radio,
        sample_per_frame, max_frame_count);
    setContentOwned(_main_component, true);

    if (isMobile())
        setFullScreen(true);
    else
    {
        setResizable(false, false);
        centreWithSize(getWidth(), getHeight());
    }

    setVisible(true);
}

void AudioApp::MainWindow::closeButtonPressed()
{
    _main_component->shutdown();
    printf ("Finish shutdown\n");
    delete _main_component;
    printf ("Finish delete _main_component");
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

juce::Colour AudioApp::MainWindow::getBackgroundColour()
{
    return juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
        ResizableWindow::backgroundColourId);
}

} // namespace GuiApp