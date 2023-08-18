#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QSound>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QSystemTrayIcon trayIcon(QIcon(":/icon.png"));
    QMenu *trayMenu = new QMenu;
    
    QAction *showAction = trayMenu->addAction("Показать");
    QAction *quitAction = trayMenu->addAction("Выход");
    
    QSound notificationSound(":/sounds/notification_sound.wav");

    QObject::connect(showAction, &QAction::triggered, &app, [&]() {
        // Действие при выборе "Показать"
        // Допустим, вы хотите показать главное окно приложения
    });

    QObject::connect(quitAction, &QAction::triggered, &app, [&]() {
        // Действие при выборе "Выход"
        app.quit();
    });

    trayIcon.setContextMenu(trayMenu);
    trayIcon.show();

    // Отображаем всплывающее сообщение и воспроизводим звук
    trayIcon.showMessage("Заголовок", "Сообщение из трея", QSystemTrayIcon::Information, 5000);
    notificationSound.play();

    return app.exec();
}
