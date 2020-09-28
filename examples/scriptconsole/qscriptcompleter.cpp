#include "qscriptcompleter.h"

#include <QDebug>
#include <QStringListModel>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

QScriptCompleter::QScriptCompleter()
{
    // https://developer.mozilla.org/en/JavaScript/Reference/Reserved_Words
    m_keywords << "break";
    m_keywords << "case";
    m_keywords << "catch";
    m_keywords << "continue";
    m_keywords << "default";
    m_keywords << "delete";
    m_keywords << "do";
    m_keywords << "else";
    m_keywords << "finally";
    m_keywords << "for";
    m_keywords << "function";
    m_keywords << "if";
    m_keywords << "in";
    m_keywords << "instanceof";
    m_keywords << "new";
    m_keywords << "return";
    m_keywords << "switch";
    m_keywords << "this";
    m_keywords << "throw";
    m_keywords << "try";
    m_keywords << "typeof";
    m_keywords << "var";
    m_keywords << "void";
    m_keywords << "while";
    m_keywords << "with";

    m_keywords << "true";
    m_keywords << "false";
    m_keywords << "null";
}

int QScriptCompleter::updateCompletionModel(const QString& code)
{
    // Start by clearing the model
    this->setModel(0);

    // Don't try to complete the empty string
    if (code.isEmpty())
    {
      return 0;
    }

    // Search backward through the string for usable characters
    QString textToComplete;
    for (int i = code.length() - 1; i >= 0; --i)
    {
      QChar c = code.at(i);
      if (c.isLetterOrNumber() || c == '.' || c == '_')
      {
        textToComplete.prepend(c);
      }
      else
      {
        break;
      }
    }

    // Split the string at the last dot, if one exists
    QString lookup;
    QString compareText = textToComplete;
    int dot = compareText.lastIndexOf('.');
    if (dot != -1)
    {
      lookup = compareText.mid(0, dot);
      compareText = compareText.mid(dot + 1);
    }

    // Lookup QtScript names
    QStringList found;
    if (!lookup.isEmpty() || !compareText.isEmpty())
    {
      compareText = compareText.toLower();
      QStringList l = introspection(lookup);
      foreach (QString n, l)
        if (n.toLower().startsWith(compareText)) found << n;
    }
    qDebug() << "lookup : " << lookup;
    qDebug() << "compareText : " << compareText;
    qDebug() << "found : " << found.size();

    // Initialize the completion model
    if (!found.isEmpty())
    {
      setCompletionMode(QCompleter::PopupCompletion);
      setModel(new QStringListModel(found, this));
      setCaseSensitivity(Qt::CaseInsensitive);
      setCompletionPrefix(compareText.toLower());
      //if (popup())
        //popup()->setCurrentIndex(completionModel()->index(0, 0));
    }

    return found.size();
}

QStringList QScriptCompleter::introspection(const QString &lookup)
{
    // list of found tokens
    QStringList properties, children, functions;

    if (!eng) return properties;

    QScriptValue scriptObj;
    if (lookup.isEmpty()) {
        properties = m_keywords;
        scriptObj = eng->globalObject();
    }
    else {
        scriptObj = eng->evaluate(lookup);
        // if the engine cannot recognize the variable return
        if (eng->hasUncaughtException()) return properties;
    }

     // if a QObject add the named children
    if (scriptObj.isQObject())
    {
        QObject* obj = scriptObj.toQObject();

        foreach(QObject* ch, obj->children())
        {
            QString name = ch->objectName();
            if (!name.isEmpty())
                children << name;
        }

    }

    // add the script properties
    {
        QScriptValue obj(scriptObj); // the object to iterate over
        while (obj.isObject()) {
            QScriptValueIterator it(obj);
            while (it.hasNext()) {
                it.next();

                // avoid array indices
                bool isIdx;
                it.scriptName().toArrayIndex(&isIdx);
                if (isIdx) continue;

                // avoid "hidden" properties starting with "__"
                if (it.name().startsWith("__")) continue;

                // include in list
                if (it.value().isQObject()) children << it.name();
                else if (it.value().isFunction()) functions << it.name();
                else properties << it.name();
            }
            obj = obj.prototype();
        }
    }

    children.removeDuplicates();
    children.sort(Qt::CaseInsensitive);
    functions.removeDuplicates();
    functions.sort(Qt::CaseInsensitive);
    properties.removeDuplicates();
    properties.sort(Qt::CaseInsensitive);

    children.append(properties);
    children.append(functions);

    return children;

}
