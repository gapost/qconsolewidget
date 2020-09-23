#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H

#include <QObject>

class QIODevice;
class QTimer;
class QScriptEngine;
class QConsoleWidget;

class ScriptObject : public QObject
{
    Q_OBJECT
public:
    explicit ScriptObject(QConsoleWidget *w);

    QTimer* waitTimer() { return waitTimer_; }
    QIODevice* device() { return iodev_; }

public slots:
    void evalCommand();
    void abortEvaluation();

private:
    QIODevice* iodev_;
    QTimer* waitTimer_;
    QConsoleWidget* w;
    QScriptEngine* e;
    QString multilineCode_;
};

#endif // SCIPTOBJECT_H
