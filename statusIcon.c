void SystemTray::connectionStatusASUV(bool connectedASUV, bool connectedPOVZ )
{
    qDebug() <<"Start" <<connectedASUV <<connectedPOVZ;
    movie->start();
    timer->start(40);
    QTimer::singleShot(4000, this, [=](){
    playerStop();
    if (connectedASUV && connectedPOVZ ){
        qDebug() <<"connectedASUV && connectedPOVZ" <<connectedASUV <<connectedPOVZ;
        QIcon icon(":tray_ok");
        trayIcon->setIcon(icon);
    }  else if (connectedASUV && !connectedPOVZ){
        qDebug() <<"connectedASUV && !connectedPOVZ" <<connectedASUV <<connectedPOVZ;
        QIcon icon(":tray_s");
        trayIcon->setIcon(icon);
    } else if (!connectedASUV && !connectedPOVZ ){
         qDebug() <<"!connectedASUV && !connectedPOVZ" <<connectedASUV <<connectedPOVZ;
        QIcon icon(":red");
        trayIcon->setIcon(icon);
    } else if (!connectedASUV && connectedPOVZ ){
        qDebug() <<"!connectedASUV && connectedPOVZ" <<connectedASUV <<connectedPOVZ;
        QIcon icon(":red");
        trayIcon->setIcon(icon);
    }

    trayIcon->show();
    });
}
