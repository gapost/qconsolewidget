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
    so->device()->write(msg.toLatin1());
    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue wait(QScriptContext *context, QScriptEngine *engine)
{
    ScriptObject* so = qobject_cast<ScriptObject*>(engine->parent());
    int ms = context->argument(0).toUInt32();
    so->waitTimer()->start(ms);

    do
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 200);
    while (so->waitTimer()->isActive());

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

    waitTimer_ = new QTimer(this);
    waitTimer_->setSingleShot(true);

    w = aw;
    iodev_ = w->device();
    iodev_->open(QIODevice::ReadWrite);
    connect(iodev_,SIGNAL(readyRead()),this,SLOT(evalCommand()));
    connect(w,SIGNAL(abortEvaluation()),this,SLOT(abortEvaluation()));

    w->writeStdOut(
                "QConsoleWidget example: qscript console\n"
                "  interactive qscript interpreter\n\n"
                "Additional commands:\n"
                "  - quit() or exit() to leave\n"
                "  - log(var x) to print x.toString()\n"
                "  - wait(ms) blocks qscript execution for given ms\n\n"
                );
    w->writeStdOut("Enter quit() or exit() to leave\n\n");
    w->writeStdOut("qs> ");
    w->setMode(QConsoleWidget::Input);
}

void ScriptObject::evalCommand()
{
    multilineCode_+= iodev_->readAll();

    if (!multilineCode_.isEmpty()) {
        if (e->canEvaluate(multilineCode_)) {
            QScriptValue ret = e->evaluate(multilineCode_);
            multilineCode_ = "";
            QTextStream os(iodev_);
            if (e->hasUncaughtException()) {
                iodev_->setCurrentWriteChannel(QConsoleWidget::StandardError);
                if (!ret.isUndefined()) os << ret.toString();
                os << endl;
                os << e->uncaughtExceptionBacktrace().join("\n") << endl;
                iodev_->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
            } else if (ret.isValid() && !ret.isUndefined()) os << ret.toString() << endl;
        } else {
            // incomplete code
            w->writeStdOut("....> ");
            w->setMode(QConsoleWidget::Input);
            return;

        }

    }

    w->writeStdOut("qs> ");
    w->setMode(QConsoleWidget::Input);
    return;
}

void ScriptObject::abortEvaluation()
{
    waitTimer_->stop();
    e->abortEvaluation();
}
