#ifndef SCRIPTSESSION_H
#define SCRIPTSESSION_H

#include <QObject>

class QElapsedTimer;
class QScriptEngine;
class QConsoleWidget;

class ScriptSession : public QObject
{
    Q_OBJECT
public:
    explicit ScriptSession(QConsoleWidget *w);

    QConsoleWidget* widget() { return w; }
    void quit() { quit_ = true; }

    void tic();
    qreal toc();

public slots:
    void REPL();

private slots:
    void abortEvaluation();

private:
    QConsoleWidget* w;
    QScriptEngine* e;
    QElapsedTimer* tmr_;
    bool quit_;
};

#endif // SCRIPTSESSION_H
