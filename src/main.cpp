#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

#include <winrt/base.h>

#include "BleScanner.h"
#include "AirPodsParser.h"
#include "AirPodsModel.h"
#include "MediaController.h"

namespace {
QIcon makeTrayIcon() {
    // ponytail: placeholder dot so the app runs with zero bundled assets -
    // swap for real tray artwork whenever it's ready.
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawEllipse(4, 4, 24, 24);
    return QIcon(pixmap);
}
}

int main(int argc, char* argv[]) {
    // MTA: BluetoothLEAdvertisementWatcher callbacks arrive on thread-pool
    // threads, not the Qt/GUI thread.
    winrt::init_apartment();

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); // keep running in the tray with no window open

    AirPodsModel model;
    MediaController mediaController;
    QObject::connect(&model, &AirPodsModel::earStateChanged,
                      &mediaController, &MediaController::onEarStateChanged);

    BleScanner scanner;
    QObject::connect(&scanner, &BleScanner::appleAdvertisementReceived, &model,
                      [&model](const QByteArray& data, const QString& address, int /*rssi*/) {
                          if (auto status = AirPodsParser::parse(data))
                              model.ingest(*status, address);
                      });
    scanner.start();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("airpodsModel", &model);
    engine.loadFromModule("WinPods", "Popup");
    if (engine.rootObjects().isEmpty())
        return -1;

    QMenu menu;
    QAction* quitAction = menu.addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

    QSystemTrayIcon tray(makeTrayIcon());
    tray.setContextMenu(&menu); // right-click: Quit menu
    tray.setToolTip("WinPods");
    QObject::connect(&tray, &QSystemTrayIcon::activated, &model,
                      [&model](QSystemTrayIcon::ActivationReason reason) {
                          // Left-click/double-click: show the popup on demand,
                          // same slide-up/auto-hide behavior as a BLE-triggered one.
                          if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                              model.togglePopup();
                      });
    tray.show();

    return app.exec();
}
