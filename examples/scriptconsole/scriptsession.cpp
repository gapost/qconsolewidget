#include "scriptsession.h"
#include "QConsoleWidget.h"

#include <QApplication>
#include <QEventLoop>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QTimer>
#include <QElapsedTimer>
#include <QIODevice>

QScriptValue quitfunc(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    ScriptSession* session = qobject_cast<ScriptSession*>(engine->parent());
    session->quit();
    qApp->quit();
    return QScriptValue(QScriptValue::UndefinedValue);
}
QScriptValue ticfunc(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    ScriptSession* session = qobject_cast<ScriptSession*>(engine->parent());
    session->tic();
    return QScriptValue(QScriptValue::UndefinedValue);
}
QScriptValue tocfunc(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    ScriptSession* session = qobject_cast<ScriptSession*>(engine->parent());
    return QScriptValue(session->toc());
}
QScriptValue log(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount()!=1) {
        return context->throwError(QScriptContext::SyntaxError,
                            "log must be called with 1 argument\n"
                            "  Usage: log(x)");
    }
    ScriptSession* session = qobject_cast<ScriptSession*>(engine->parent());
    QString msg = context->argument(0).toString() + "\n";
    session->widget()->writeStdOut(msg.toLatin1());
    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue wait(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount()!=1) {
        return context->throwError(QScriptContext::SyntaxError,
                            "wait must be called with 1 argument\n"
                            "  Usage: wait(ms)");
    }
    ScriptSession* session = qobject_cast<ScriptSession*>(engine->parent());
    int msecs = context->argument(0).toUInt32();

    QEventLoop loop;
    QObject::connect(session->widget(),SIGNAL(abortEvaluation()),&loop,SLOT(quit()));
    QTimer::singleShot(msecs,&loop,SLOT(quit()));

    loop.exec();

    return QScriptValue(QScriptValue::UndefinedValue);
}

ScriptSession::ScriptSession(QConsoleWidget *aw) : QObject(), w(aw), quit_(false)
{
    e = new QScriptEngine(this);

    QScriptValue v;
    v = e->newFunction(quitfunc);
    e->globalObject().setProperty("quit", v);
    e->globalObject().setProperty("exit", v);

    v = e->newFunction(log);
    e->globalObject().setProperty("log", v);

    v = e->newFunction(wait);
    e->globalObject().setProperty("wait", v);

    v = e->newFunction(ticfunc);
    e->globalObject().setProperty("tic", v);

    v = e->newFunction(tocfunc);
    e->globalObject().setProperty("toc", v);

    w->device()->open(QIODevice::ReadWrite);
    connect(w,SIGNAL(abortEvaluation()),this,SLOT(abortEvaluation()));

    tmr_ = new QElapsedTimer();

}

void ScriptSession::tic()
{
    tmr_->start();
}
qreal ScriptSession::toc()
{
    return  1.e-6*tmr_->nsecsElapsed();
}

void ScriptSession::abortEvaluation()
{
    e->abortEvaluation();
}

void ScriptSession::REPL()
{
    QIODevice* d = w->device();
    QTextStream ws(d);
    QString multilineCode;

    forever {

        if (quit_) break;

        ws << (multilineCode.isEmpty() ? "qs> " : "....> ") << flush;

        ws >> inputMode >> waitForInput;

        multilineCode += ws.readAll();

        if (!multilineCode.isEmpty())
        {

            if (e->canEvaluate(multilineCode))
            {
                QScriptValue ret = e->evaluate(multilineCode);
                multilineCode = "";

                if (e->hasUncaughtException())
                {
                    ws << errChannel;
                    if (!ret.isUndefined()) ws << ret.toString();
                    ws << endl;
                    ws << e->uncaughtExceptionBacktrace().join("\n") << endl;
                    ws << outChannel;
                }
                else if (ret.isValid() && !ret.isUndefined())
                    ws << ret.toString() << endl;
            }
        }
    }

}


