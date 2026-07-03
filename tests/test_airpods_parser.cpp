#include "../src/AirPodsParser.h"
#include <cassert>
#include <cstdio>

// ponytail: plain asserts, no test framework - just enough to catch a
// nibble-indexing regression in AirPodsParser::parse.
int main() {
    // Bytes: 07 19 01 02 20 22 85 49 00*8
    //  - model      = 0220 -> "AirPods Pro"
    //  - flip       = 0 (byte5 high nibble = 0x2)
    //  - in-ear raw = 0x2  -> left in ear, right not
    //  - battery byte6 = 0x85: low nibble ("curr", byte6&0xF=5) is Left when
    //    flip==0, high nibble ("anot", byte6>>4=8) is Right - verified
    //    against AirPodsDesktop's AppleCP.h bitfield struct.
    //  - charge raw = 0x4  -> case charging only
    //  - case batt  = 0x9  (90%)
    const QByteArray raw = QByteArray::fromHex("07190102202285490000000000000000");

    auto status = AirPodsParser::parse(raw);
    assert(status.has_value());
    assert(status->model == "AirPods Pro");
    assert(status->batteryLeft == 50);
    assert(status->batteryRight == 80);
    assert(status->batteryCase == 90);
    assert(status->chargingLeft == false);
    assert(status->chargingRight == false);
    assert(status->chargingCase == true);
    assert(status->inEarLeft == true);
    assert(status->inEarRight == false);

    // Malformed/foreign input must be rejected, not crash.
    assert(!AirPodsParser::parse(QByteArray::fromHex("00112233")).has_value());
    assert(!AirPodsParser::parse(QByteArray()).has_value());

    std::printf("AirPodsParser self-test passed\n");
    return 0;
}
