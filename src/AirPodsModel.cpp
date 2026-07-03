#include "AirPodsModel.h"

AirPodsModel::AirPodsModel(QObject* parent) : QObject(parent) {
    m_autoHideTimer.setSingleShot(true);
    m_autoHideTimer.setInterval(6000);
    connect(&m_autoHideTimer, &QTimer::timeout, this, [this] { setPopupVisible(false); });

    m_staleTimer.setSingleShot(true);
    m_staleTimer.setInterval(15000); // Apple beacons repeat roughly once a second while nearby
    connect(&m_staleTimer, &QTimer::timeout, this, [this] {
        if (!m_connected) return;
        m_connected = false;
        emit connectedChanged();
        setPopupVisible(false);
    });
}

void AirPodsModel::ingest(const AirPodsStatus& status, const QString& deviceAddress) {
    const bool wasBothInCase = m_status.bothInCase;
    const bool wasEarL = m_status.inEarLeft;
    const bool wasEarR = m_status.inEarRight;
    // Don't key this off deviceAddress: AirPods have two independent BLE
    // radios (one per earbud) and Apple rotates the advertising address
    // periodically, so the "same" connected AirPods legitimately show up
    // under a different address mid-session - that used to be misread as a
    // brand new device and re-triggered the popup at random.
    const bool isNewDevice = !m_connected;

    m_status = status;
    m_lastAddress = deviceAddress;
    m_staleTimer.start();

    if (!m_connected) {
        m_connected = true;
        emit connectedChanged();
    }
    emit statusChanged();

    // Popup appears like on iOS: a fresh device shows up nearby, or the case
    // was closed (both pods seated+charging) and has now been opened again.
    if (isNewDevice || (wasBothInCase && !status.bothInCase))
        setPopupVisible(true);

    if (wasEarL != status.inEarLeft || wasEarR != status.inEarRight)
        emit earStateChanged(status.inEarLeft, status.inEarRight);
}

void AirPodsModel::setPopupVisible(bool visible) {
    if (m_popupVisible == visible) return;
    m_popupVisible = visible;
    emit popupVisibleChanged();
    if (visible) m_autoHideTimer.start();
}

void AirPodsModel::dismissPopup() {
    m_autoHideTimer.stop();
    setPopupVisible(false);
}

void AirPodsModel::togglePopup() {
    if (m_popupVisible)
        dismissPopup();
    else
        setPopupVisible(true);
}
