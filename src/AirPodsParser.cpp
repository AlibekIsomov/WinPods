#include "AirPodsParser.h"
#include <QHash>

namespace {

// Community-maintained model table (2 bytes, as they appear at payload
// offset 3-4). Not exhaustive - extend as new hardware ships.
// ponytail: a static table is fine here; promote to a data file only if
// this needs to be updated more often than the app itself.
QString modelName(quint8 b1, quint8 b2) {
    static const QHash<quint16, QString> kModels = {
        { 0x0220, QStringLiteral("AirPods Pro") },
        { 0x0E20, QStringLiteral("AirPods (1st generation)") },
        { 0x0F20, QStringLiteral("AirPods (2nd generation)") },
        { 0x1320, QStringLiteral("AirPods (3rd generation)") },
        { 0x1420, QStringLiteral("AirPods Pro (2nd generation)") },
        { 0x0A20, QStringLiteral("AirPods Max") },
        { 0x0620, QStringLiteral("Powerbeats3") },
        { 0x0320, QStringLiteral("Powerbeats Pro") },
        { 0x0520, QStringLiteral("BeatsX") },
        { 0x1020, QStringLiteral("Beats Flex") },
        { 0x0920, QStringLiteral("Beats Solo3") },
        { 0x1120, QStringLiteral("Beats Studio Buds") },
    };
    const quint16 key = (quint16(b1) << 8) | b2;
    return kModels.value(key, QStringLiteral("AirPods"));
}

} // namespace

std::optional<AirPodsStatus> AirPodsParser::parse(const QByteArray& data) {
    // Apple broadcasts at least two 0x07 sub-messages: the full status one
    // we want (length byte 0x19, ~27 bytes, carries battery/ear-detect) and a
    // shorter one (length byte 0x0F, ~17 bytes) with no battery data at these
    // offsets. Must check the length byte too, or the short one gets
    // misread as battery/charge garbage since it still passes a size check.
    if (data.size() < 16 || static_cast<uint8_t>(data[0]) != 0x07 || static_cast<uint8_t>(data[1]) != 0x19)
        return std::nullopt;

    const QByteArray hex = data.toHex().toUpper(); // 2 ASCII hex chars per byte

    // Every reference implementation (OpenPods/AirStatus/MagicPods) indexes
    // by hex-nibble position rather than byte+shift, so this mirrors that to
    // stay easy to cross-check against them.
    auto nibble = [&hex](int i) -> int {
        bool ok = false;
        const int v = QByteArray(1, hex[i]).toInt(&ok, 16);
        return ok ? v : 0;
    };

    AirPodsStatus status;
    status.model = modelName(static_cast<quint8>(data[3]), static_cast<quint8>(data[4]));

    // Bit 1 of this status nibble tells us whether the left/right nibbles
    // below are in "normal" or "flipped" order (depends on which pod is
    // currently the BLE-advertising primary).
    const int flip = (nibble(10) & 0x02) == 0 ? 1 : 0;

    auto toPercent = [](int raw) { return raw == 15 ? -1 : raw * 10; };

    // Cross-checked against AirPodsDesktop's AppleCP.h bitfield struct: byte 6
    // packs "curr" in the low nibble (hex char 13) and "anot" in the high
    // nibble (hex char 12) - this was backwards before and swapped L/R
    // whenever the two differ (identical values on both sides hid the bug).
    status.batteryLeft  = toPercent(nibble(flip == 0 ? 13 : 12));
    status.batteryRight = toPercent(nibble(flip == 0 ? 12 : 13));
    status.batteryCase  = toPercent(nibble(15));

    const int chargeRaw = nibble(14);
    status.chargingLeft  = flip == 0 ? (chargeRaw & 0b0001) != 0 : (chargeRaw & 0b0010) != 0;
    status.chargingRight = flip == 0 ? (chargeRaw & 0b0010) != 0 : (chargeRaw & 0b0001) != 0;
    status.chargingCase  = (chargeRaw & 0b0100) != 0;

    const int inEarRaw = nibble(11);
    status.inEarLeft  = flip == 0 ? (inEarRaw & 0b0010) != 0 : (inEarRaw & 0b1000) != 0;
    status.inEarRight = flip == 0 ? (inEarRaw & 0b1000) != 0 : (inEarRaw & 0b0010) != 0;

    status.bothInCase = status.chargingLeft && status.chargingRight
                         && !status.inEarLeft && !status.inEarRight;

    return status;
}
