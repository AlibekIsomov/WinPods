#include "BleScanner.h"

using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Storage::Streams;

BleScanner::BleScanner(QObject* parent) : QObject(parent) {
    m_watcher = BluetoothLEAdvertisementWatcher();
    // Active scanning gets us manufacturer-data advertisements reliably;
    // Apple's beacons don't rely on scan responses so this is just the
    // more responsive of the two modes.
    m_watcher.ScanningMode(BluetoothLEScanningMode::Active);

    m_receivedToken = m_watcher.Received(
        [this](BluetoothLEAdvertisementWatcher const& watcher,
               BluetoothLEAdvertisementReceivedEventArgs const& args) {
            onAdvertisementReceived(watcher, args);
        });
}

BleScanner::~BleScanner() {
    stop();
    m_watcher.Received(m_receivedToken);
}

void BleScanner::start() {
    if (m_watcher.Status() != BluetoothLEAdvertisementWatcherStatus::Started)
        m_watcher.Start();
}

void BleScanner::stop() {
    if (m_watcher.Status() == BluetoothLEAdvertisementWatcherStatus::Started)
        m_watcher.Stop();
}

void BleScanner::onAdvertisementReceived(
    BluetoothLEAdvertisementWatcher const&,
    BluetoothLEAdvertisementReceivedEventArgs const& args) {
    for (auto const& section : args.Advertisement().ManufacturerData()) {
        if (section.CompanyId() != kAppleCompanyId)
            continue;

        auto buffer = section.Data();
        QByteArray payload(static_cast<int>(buffer.Length()), Qt::Uninitialized);
        DataReader::FromBuffer(buffer).ReadBytes(
            winrt::array_view<uint8_t>(reinterpret_cast<uint8_t*>(payload.data()),
                                        reinterpret_cast<uint8_t*>(payload.data() + payload.size())));

        // Apple "Proximity Pairing" message always starts with type byte 0x07.
        if (payload.size() < 2 || static_cast<uint8_t>(payload[0]) != 0x07)
            continue;

        const uint64_t addr = args.BluetoothAddress();
        const QString address = QStringLiteral("%1:%2:%3:%4:%5:%6")
            .arg(uint8_t(addr >> 40), 2, 16, QLatin1Char('0'))
            .arg(uint8_t(addr >> 32), 2, 16, QLatin1Char('0'))
            .arg(uint8_t(addr >> 24), 2, 16, QLatin1Char('0'))
            .arg(uint8_t(addr >> 16), 2, 16, QLatin1Char('0'))
            .arg(uint8_t(addr >> 8), 2, 16, QLatin1Char('0'))
            .arg(uint8_t(addr), 2, 16, QLatin1Char('0'))
            .toUpper();

        emit appleAdvertisementReceived(payload, address, args.RawSignalStrengthInDBm());
    }
}
