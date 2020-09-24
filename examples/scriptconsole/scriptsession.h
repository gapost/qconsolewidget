#ifndef SCRIPTSESSION_H
#define SCRIPTSESSION_H

#include <QObject>

class QTimer;
class QScriptEngine;
class QConsoleWidget;

class ScriptSession : public QObject
{
    Q_OBJECT
public:
    explicit ScriptSession(QConsoleWidget *w);

    QConsoleWidget* widget() { return w; }

public slots:
    void REPL();

private slots:
    void abortEvaluation();

private:
    QConsoleWidget* w;
    QScriptEngine* e;
};

#endif // SCRIPTSESSION_H
