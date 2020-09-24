#include "scriptsession.h"
#include "QConsoleWidget.h"

#include <QApplication>
#include <QEventLoop>
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
    ScriptSession* so = qobject_cast<ScriptSession*>(engine->parent());
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
    ScriptSession* so = qobject_cast<ScriptSession*>(engine->parent());
    int msecs = context->argument(0).toUInt32();

    QEventLoop loop;
    QObject::connect(so->widget(),SIGNAL(abortEvaluation()),&loop,SLOT(quit()));
    QTimer::singleShot(msecs,&loop,SLOT(quit()));

    loop.exec();

    return QScriptValue(QScriptValue::UndefinedValue);
}

ScriptSession::ScriptSession(QConsoleWidget *aw) : QObject(), w(aw)
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

    w->device()->open(QIODevice::ReadWrite);
    connect(w,SIGNAL(abortEvaluation()),this,SLOT(abortEvaluation()));

}

void ScriptSession::abortEvaluation()
{
    e->abortEvaluation();
}

void ScriptSession::REPL()
{
    QIODevice* d = w->device();
    QTextStream os(d);
    QString multilineCode;

    forever {

        os << (multilineCode.isEmpty() ? "qs> " : "....> ") << flush;
        w->setMode(QConsoleWidget::Input);
        if (!d->waitForReadyRead(-1)) break;

        multilineCode+= os.readAll();

        if (!multilineCode.isEmpty()) {
            if (e->canEvaluate(multilineCode)) {
                QScriptValue ret = e->evaluate(multilineCode);
                multilineCode = "";
                if (e->hasUncaughtException()) {
                    d->setCurrentWriteChannel(QConsoleWidget::StandardError);
                    if (!ret.isUndefined()) os << ret.toString();
                    os << endl;
                    os << e->uncaughtExceptionBacktrace().join("\n") << endl;
                    d->setCurrentWriteChannel(QConsoleWidget::StandardOutput);
                } else if (ret.isValid() && !ret.isUndefined()) os << ret.toString() << endl;

            }
        }
    }

}


