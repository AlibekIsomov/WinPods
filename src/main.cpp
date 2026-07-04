#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>

#include <winrt/base.h>

#include "BleScanner.h"
#include "AirPodsParser.h"
#include "AirPodsModel.h"
#include "MediaController.h"

int main(int argc, char* argv[]) {
    // STA: Qt's Windows integration calls OleInitialize, which requires the
    // GUI thread to be single-threaded apartment (MTA here aborts startup with
    // "Cannot change thread mode after it is set"). The BLE watcher doesn't
    // care - its Received callbacks arrive on thread-pool threads either way.
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); // keep running in the tray with no window open

    const QIcon appIcon(":/resources/AirPods.ico");
    app.setWindowIcon(appIcon);

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

    QSystemTrayIcon tray(appIcon);
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
