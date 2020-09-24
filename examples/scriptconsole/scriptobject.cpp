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
    if (context->argumentCount()!=1) {
        return context->throwError(QScriptContext::SyntaxError,
                            "log must be called with 1 argument\n"
                            "  Usage: log(x)");
    }
    ScriptObject* so = qobject_cast<ScriptObject*>(engine->parent());
    QString msg = context->argument(0).toString() + "\n";
    so->widget()->writeStdOut(msg.toLatin1());
    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue wait(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount()!=1) {
        return context->throwError(QScriptContext::SyntaxError,
                            "wait must be called with 1 argument\n"
                            "  Usage: wait(ms)");
    }
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
    w->device()->open(QIODevice::ReadWrite);

    connect(w->device(),SIGNAL(readyRead()),this,SLOT(evalCommand()));
    connect(w,SIGNAL(abortEvaluation()),this,SLOT(abortEvaluation()));

}

void ScriptObject::evalCommand()
{
    QIODevice* d = w->device();
    QTextStream os(d);

    multilineCode_+= d->readAll();

    if (!multilineCode_.isEmpty()) {
        if (e->canEvaluate(multilineCode_)) {
            QScriptValue ret = e->evaluate(multilineCode_);
            multilineCode_ = "";
            if (e->hasUncaughtException()) {
                d->setCurrentWriteChannel(QConsoleWidget::StandardError);
                if (!ret.isUndefined()) os << ret.toString();
                os << endl;
                os << e->uncaughtExceptionBacktrace().join("\n") << endl;
                d->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
            } else if (ret.isValid() && !ret.isUndefined()) os << ret.toString() << endl;
        } else {
            // incomplete code
            os << "....> " << flush;
            w->setMode(QConsoleWidget::Input);
            return;
        }
    }

    os << "qs> " << flush;
    w->setMode(QConsoleWidget::Input);
    return;
}

void ScriptObject::abortEvaluation()
{
    waitTimer_->stop();
    e->abortEvaluation();
}


