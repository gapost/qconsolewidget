#include "QConsoleWidget.h"
#include "scriptobject.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QConsoleWidget w;
    ScriptObject qs(&w);

    w.setWindowTitle("qscript console");
#if defined(Q_OS_MAC)
    w.setFont(QFont("Monaco"));
#elif defined(Q_OS_UNIX)
    w.setFont(QFont("Monospace"));
#elif defined(Q_OS_WIN)
    w.setFont(QFont("Courier New"));
#endif
    w.writeStdOut(
                "QConsoleWidget example:"
                " interactive qscript interpreter\n\n"
                "Additional commands:\n"
                "  - quit()   : end program\n"
                "  - exit()   : same\n"
                "  - log(x)   : printout x.toString()\n"
                "  - wait(ms) : block qscript execution for given ms\n\n"
                );
    w.writeStdOut("qs> ");
    w.setMode(QConsoleWidget::Input);
    w.show();

    return a.exec();
}
