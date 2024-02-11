void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QFile file("mylog.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    switch (type) {
    case QtDebugMsg:
        out << "Debug: " << context.file << ":" << context.line << " - " << msg << endl;
        break;
    case QtInfoMsg:
        out << "Info: " << context.file << ":" << context.line << " - " << msg << endl;
        break;
    case QtWarningMsg:
        out << "Warning: " << context.file << ":" << context.line << " - " << msg << endl;
        break;
    case QtCriticalMsg:
        out << "Critical: " << context.file << ":" << context.line << " - " << msg << endl;
        break;
    case QtFatalMsg:
        out << "Fatal: " << context.file << ":" << context.line << " - " << msg << endl;
        abort();
    }
}
