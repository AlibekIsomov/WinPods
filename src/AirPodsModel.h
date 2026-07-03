#pragma once

#include <QObject>
#include <QTimer>
#include <QString>

#include "AirPodsParser.h"

// Single global instance, exposed to QML as a context property
// ("airpodsModel" - see main.cpp). Owns the decoded state plus the
// show/auto-hide logic for the popup window.
class AirPodsModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY statusChanged)
    Q_PROPERTY(int batteryLeft READ batteryLeft NOTIFY statusChanged)
    Q_PROPERTY(int batteryRight READ batteryRight NOTIFY statusChanged)
    Q_PROPERTY(int batteryCase READ batteryCase NOTIFY statusChanged)
    Q_PROPERTY(bool chargingLeft READ chargingLeft NOTIFY statusChanged)
    Q_PROPERTY(bool chargingRight READ chargingRight NOTIFY statusChanged)
    Q_PROPERTY(bool chargingCase READ chargingCase NOTIFY statusChanged)
    Q_PROPERTY(bool inEarLeft READ inEarLeft NOTIFY statusChanged)
    Q_PROPERTY(bool inEarRight READ inEarRight NOTIFY statusChanged)
    Q_PROPERTY(bool popupVisible READ popupVisible NOTIFY popupVisibleChanged)

public:
    explicit AirPodsModel(QObject* parent = nullptr);

    // Called from main.cpp whenever the parser produces a fresh status.
    void ingest(const AirPodsStatus& status, const QString& deviceAddress);

    bool connected() const { return m_connected; }
    QString modelName() const { return m_status.model; }
    int batteryLeft() const { return m_status.batteryLeft; }
    int batteryRight() const { return m_status.batteryRight; }
    int batteryCase() const { return m_status.batteryCase; }
    bool chargingLeft() const { return m_status.chargingLeft; }
    bool chargingRight() const { return m_status.chargingRight; }
    bool chargingCase() const { return m_status.chargingCase; }
    bool inEarLeft() const { return m_status.inEarLeft; }
    bool inEarRight() const { return m_status.inEarRight; }
    bool popupVisible() const { return m_popupVisible; }

    Q_INVOKABLE void dismissPopup();
    Q_INVOKABLE void togglePopup(); // tray-icon click: show it, or hide it if already open

signals:
    void connectedChanged();
    void statusChanged();
    void popupVisibleChanged();
    void earStateChanged(bool leftInEar, bool rightInEar); // consumed by MediaController

private:
    void setPopupVisible(bool visible);

    AirPodsStatus m_status;
    QString m_lastAddress;
    bool m_connected = false;
    bool m_popupVisible = false;

    QTimer m_autoHideTimer; // popup dismisses itself after a few seconds, like iOS
    QTimer m_staleTimer;    // no advert for a while -> treat as disconnected/out of range
};
