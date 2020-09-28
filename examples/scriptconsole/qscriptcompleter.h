#ifndef QSCRIPTCOMPLETER_H
#define QSCRIPTCOMPLETER_H

#include "QConsoleWidget.h"

class QScriptEngine;

class QScriptCompleter : public QConsoleWidgetCompleter
{
public:
    QScriptCompleter();
    virtual int updateCompletionModel(const QString& code);

    void seScripttEngine(QScriptEngine* e)
    { eng = e; }

private:
    QStringList introspection(const QString& lookup);
    QScriptEngine* eng;
    QStringList m_keywords;
};

#endif // QSCRIPTCOMPLETER_H
