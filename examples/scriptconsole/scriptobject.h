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

    QIODevice* iodev;
    QTimer* waitTimer;

signals:

public slots:
    void evalCommand();
    void abortEvaluation();

private:
    QConsoleWidget* w;
    QScriptEngine* e;
};

#endif // SCIPTOBJECT_H
