#pragma once

#include <QObject>

// Bridges in-ear detection to Windows' System Media Transport Controls:
// pause whatever is playing when both pods leave the ear, resume it when a
// pod goes back in - the same behavior AirPods have on iOS/macOS.
class MediaController : public QObject {
    Q_OBJECT
public:
    explicit MediaController(QObject* parent = nullptr);

public slots:
    void onEarStateChanged(bool leftInEar, bool rightInEar);

private:
    void pauseActiveSession();
    void playActiveSession();

    bool m_pausedByUs = false;
};
