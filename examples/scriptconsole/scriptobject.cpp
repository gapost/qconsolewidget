#include "scriptobject.h"
#include "QConsoleWidget.h"

#include <QApplication>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QTimer>
#include <QIODevice>

QScriptValue quit(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Q_UNUSED(engine);
    qApp->quit();
    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue log(QScriptContext *context, QScriptEngine *engine)
{
    ScriptObject* so = qobject_cast<ScriptObject*>(engine->parent());
    QString msg = context->argument(0).toString() + "\n";
    so->iodev->write(msg.toLatin1());
    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue wait(QScriptContext *context, QScriptEngine *engine)
{
    ScriptObject* so = qobject_cast<ScriptObject*>(engine->parent());
    int ms = context->argument(0).toUInt32();
    so->waitTimer->start(ms);

    do
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 200);
    while (so->waitTimer->isActive());

    return QScriptValue(QScriptValue::UndefinedValue);
}

ScriptObject::ScriptObject(QConsoleWidget *aw) : QObject()
{
    e = new QScriptEngine(this);

    QScriptValue v;
    v = e->newFunction(quit);
    e->globalObject().setProperty("quit", v);
    e->globalObject().setProperty("exit", v);

    v = e->newFunction(log);
    e->globalObject().setProperty("log", v);

    v = e->newFunction(wait);
    e->globalObject().setProperty("wait", v);

    waitTimer = new QTimer(this);
    waitTimer->setSingleShot(true);

    w = aw;
    iodev = w->device();
    iodev->open(QIODevice::ReadWrite);
    connect(iodev,SIGNAL(readyRead()),this,SLOT(evalCommand()));
    connect(w,SIGNAL(abortEvaluation()),this,SLOT(abortEvaluation()));

    w->writeStdOut("QConsoleWidget example: qscript console\n\n");
    w->writeStdOut("Enter quit() or exit() to leave\n\n");
    w->writeStdOut(">> ");
    w->setMode(QConsoleWidget::Input);
}

void ScriptObject::evalCommand()
{

    QByteArray code = iodev->readAll();
    if (code.endsWith('\n')) code.chop(1);

    if (!code.isEmpty()) {
        QScriptValue ret = e->evaluate(code);
        QTextStream os(iodev);
        if (e->hasUncaughtException()) {
            iodev->setCurrentWriteChannel(QConsoleWidget::StandardError);
            if (!ret.isUndefined()) os << ret.toString();
            os << endl;
            os << e->uncaughtExceptionBacktrace().join("\n") << endl;
            iodev->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
        } else if (ret.isValid() && !ret.isUndefined()) os << ret.toString() << endl;

    }

    w->writeStdOut(">> ");
    w->setMode(QConsoleWidget::Input);
    return;
}

void ScriptObject::abortEvaluation()
{
    waitTimer->stop();
    e->abortEvaluation();
}
