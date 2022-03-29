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
    size_t sample_per_frame = 8192;
    size_t max_frame_count = 128;
    _main_component = new MainComponent(sample_per_frame, max_frame_count);
    setContentOwned(_main_component, true);

    if (isMobile())
        setFullScreen(true);
    else
    {
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
    }

    setVisible(true);
}

void AudioApp::MainWindow::closeButtonPressed()
{
    _main_component->shutdown();
    delete _main_component;
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

juce::Colour AudioApp::MainWindow::getBackgroundColour()
{
    return juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
        ResizableWindow::backgroundColourId);
}

} // namespace GuiApp