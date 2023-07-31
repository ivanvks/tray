class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();

private:
    void createIconGroupBox();
    void createMessageGroupBox();
    void createActions();
    void createTrayIcon();

    QGroupBox *iconGroupBox;
    QLabel *iconLabel;
    QComboBox *iconComboBox;
    QCheckBox *showIconCheckBox;

    QGroupBox *messageGroupBox;
    QLabel *typeLabel;
    QLabel *durationLabel;
    QLabel *durationWarningLabel;
    QLabel *titleLabel;
    QLabel *bodyLabel;
    QComboBox *typeComboBox;
    QSpinBox *durationSpinBox;
    QLineEdit *titleEdit;
    QTextEdit *bodyEdit;
    QPushButton *showMessageButton;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};
We implement several private slots to respond to user interaction. The other private functions are only convenience functions provided to simplify the constructor.

The tray icon is an instance of the QSystemTrayIcon class. To check whether a system tray is present on the user's desktop, call the static QSystemTrayIcon::isSystemTrayAvailable() function. Associated with the icon, we provide a menu containing the typical minimize, maximize, restore and quit actions. We reimplement the QWidget::setVisible() function to update the tray icon's menu whenever the editor's appearance changes, e.g., when maximizing or minimizing the main application window.

Finally, we reimplement QWidget's closeEvent() function to be able to inform the user (when closing the editor window) that the program will keep running in the system tray until the user chooses the Quit entry in the icon's context menu.

Window Class Implementation
When constructing the editor widget, we first create the various editor elements before we create the actual system tray icon:

Window::Window()
{
    createIconGroupBox();
    createMessageGroupBox();

    iconLabel->setMinimumWidth(durationLabel->sizeHint().width());

    createActions();
    createTrayIcon();

    connect(showMessageButton, &QAbstractButton::clicked, this, &Window::showMessage);
    connect(showIconCheckBox, &QAbstractButton::toggled, trayIcon, &QSystemTrayIcon::setVisible);
    connect(iconComboBox, &QComboBox::currentIndexChanged,
            this, &Window::setIcon);
    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &Window::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconGroupBox);
    mainLayout->addWidget(messageGroupBox);
    setLayout(mainLayout);

    iconComboBox->setCurrentIndex(1);
    trayIcon->show();

    setWindowTitle(tr("Systray"));
    resize(400, 300);
}
We ensure that the application responds to user input by connecting most of the editor's input widgets (including the system tray icon) to the application's private slots. But note the visibility checkbox; its toggled() signal is connected to the icon's setVisible() function instead.

void Window::setIcon(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip(iconComboBox->itemText(index));
}
The setIcon() slot is triggered whenever the current index in the icon combobox changes, i.e., whenever the user chooses another icon in the editor. Note that it is also called when the user activates the tray icon with the left mouse button, triggering the icon's activated() signal. We will come back to this signal shortly.

The QSystemTrayIcon::setIcon() function sets the icon property that holds the actual system tray icon. On Windows, the system tray icon size is 16x16; on X11, the preferred size is 22x22. The icon will be scaled to the appropriate size as necessary.

Note that on X11, due to a limitation in the system tray specification, mouse clicks on transparent areas in the icon are propagated to the system tray. If this behavior is unacceptable, we suggest using an icon with no transparency.

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
        break;
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    default:
        ;
    }
}
Whenever the user activates the system tray icon, it emits its activated() signal passing the triggering reason as parameter. QSystemTrayIcon provides the ActivationReason enum to describe how the icon was activated.

In the constructor, we connected our icon's activated() signal to our custom iconActivated() slot: If the user has clicked the icon using the left mouse button, this function changes the icon image by incrementing the icon combobox's current index, triggering the setIcon() slot as mentioned above. If the user activates the icon using the middle mouse button, it calls the custom showMessage() slot:

void Window::showMessage()
{
    showIconCheckBox->setChecked(true);
    int selectedIcon = typeComboBox->itemData(typeComboBox->currentIndex()).toInt();
    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(selectedIcon);

    if (selectedIcon == -1) { // custom icon
        QIcon icon(iconComboBox->itemIcon(iconComboBox->currentIndex()));
        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), icon,
                          durationSpinBox->value() * 1000);
    } else {
        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), msgIcon,
                          durationSpinBox->value() * 1000);
    }
}
When the showMessage() slot is triggered, we first retrieve the message icon depending on the currently chosen message type. The QSystemTrayIcon::MessageIcon enum describes the icon that is shown when a balloon message is displayed. Then we call QSystemTrayIcon's showMessage() function to show the message with the title, body, and icon for the time specified in milliseconds.

macOS users note: The Growl notification system must be installed for QSystemTrayIcon::showMessage() to display messages.

QSystemTrayIcon also has the corresponding, messageClicked() signal, which is emitted when the user clicks a message displayed by showMessage().

void Window::messageClicked()
{
    QMessageBox::information(nullptr, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
}
In the constructor, we connected the messageClicked() signal to our custom messageClicked() slot that simply displays a message using the QMessageBox class.

QMessageBox provides a modal dialog with a short message, an icon, and buttons laid out depending on the current style. It supports four severity levels: "Question", "Information", "Warning" and "Critical". The easiest way to pop up a message box in Qt is to call one of the associated static functions, e.g., QMessageBox::information().

As we mentioned earlier, we reimplement a couple of QWidget's virtual functions:

void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}
Our reimplementation of the QWidget::setVisible() function updates the tray icon's menu whenever the editor's appearance changes, e.g., when maximizing or minimizing the main application window, before calling the base class implementation.

void Window::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible())
        return;
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}
We have reimplemented the QWidget::closeEvent() event handler to receive widget close events, showing the above message to the users when they are closing the editor window. We need to avoid showing the message and accepting the close event when the user really intends to quit the application: that is, when the user has triggered "Quit" in the menu bar, or in the tray icon's context menu, or pressed Command+Q shortcut on macOS.

In addition to the functions and slots discussed above, we have also implemented several convenience functions to simplify the constructor: createIconGroupBox(), createMessageGroupBox(), createActions() and createTrayIcon(). See the desktop/systray/window.cpp file for details
