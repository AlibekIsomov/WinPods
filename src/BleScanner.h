#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <cstdint>

// Must come before the Advertisement header: on some SDK/MSVC combinations,
// IVector<BluetoothLEManufacturerData>'s Size()/GetAt()/range-for fail to
// compile (C3779) unless Collections.h has already been parsed first.
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Storage.Streams.h>

// Listens for BLE advertisements and forwards only Apple (company ID 0x004C)
// manufacturer-data payloads. Runs on WinRT's thread pool; Qt marshals the
// signal to whatever thread the receiver lives on.
class BleScanner : public QObject {
    Q_OBJECT
public:
    explicit BleScanner(QObject* parent = nullptr);
    ~BleScanner() override;

    void start();
    void stop();

signals:
    void appleAdvertisementReceived(QByteArray manufacturerData, QString deviceAddress, int rssi);

private:
    void onAdvertisementReceived(
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& watcher,
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);

    winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher m_watcher{ nullptr };
    winrt::event_token m_receivedToken{};

    static constexpr uint16_t kAppleCompanyId = 0x004C;
};
