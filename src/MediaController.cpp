#include "MediaController.h"

#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.h>

using namespace winrt::Windows::Media::Control;
using namespace winrt::Windows::Foundation;

MediaController::MediaController(QObject* parent) : QObject(parent) {}

void MediaController::onEarStateChanged(bool leftInEar, bool rightInEar) {
    const bool bothOut = !leftInEar && !rightInEar;
    if (bothOut)
        pauseActiveSession();
    else if (m_pausedByUs)
        playActiveSession();
}

void MediaController::pauseActiveSession() {
    GlobalSystemMediaTransportControlsSessionManager::RequestAsync().Completed(
        [this](IAsyncOperation<GlobalSystemMediaTransportControlsSessionManager> const& op, AsyncStatus status) {
            if (status != AsyncStatus::Completed) return;
            auto session = op.GetResults().GetCurrentSession();
            if (!session) return;

            if (session.GetPlaybackInfo().PlaybackStatus()
                == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) {
                session.TryPauseAsync(); // fire-and-forget: we don't need the result
                m_pausedByUs = true;
            }
        });
}

void MediaController::playActiveSession() {
    GlobalSystemMediaTransportControlsSessionManager::RequestAsync().Completed(
        [this](IAsyncOperation<GlobalSystemMediaTransportControlsSessionManager> const& op, AsyncStatus status) {
            if (status != AsyncStatus::Completed) return;
            if (auto session = op.GetResults().GetCurrentSession())
                session.TryPlayAsync();
            m_pausedByUs = false;
        });
}
