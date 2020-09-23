#include "QConsoleWidget.h"
#include "scriptobject.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QConsoleWidget w;
    w.setWindowTitle("qscript console");
    w.show();

    ScriptObject qs(&w);

    return a.exec();
}
