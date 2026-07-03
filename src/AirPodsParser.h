#pragma once

#include <QByteArray>
#include <QString>
#include <optional>

struct AirPodsStatus {
    QString model;

    int batteryLeft  = -1; // 0-100 in steps of 10, -1 = unavailable/disconnected
    int batteryRight = -1;
    int batteryCase  = -1;

    bool chargingLeft  = false;
    bool chargingRight = false;
    bool chargingCase  = false;

    bool inEarLeft  = false;
    bool inEarRight = false;

    bool bothInCase = false; // both pods seated and charging -> case is (still) closed/idle
};

// Decodes Apple's "Proximity Pairing" (0x07) manufacturer-data message
// broadcast by AirPods and compatible Beats headsets. The byte layout is not
// publicly documented by Apple; this follows the format reverse-engineered
// and cross-validated by the OpenPods / AirStatus / MagicPods projects.
class AirPodsParser {
public:
    static std::optional<AirPodsStatus> parse(const QByteArray& manufacturerData);
};
