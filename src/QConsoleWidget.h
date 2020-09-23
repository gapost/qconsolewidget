#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include <QPlainTextEdit>
#include <QTextStream>

class QCompleter;
class QConsoleIODevice;

class QConsoleWidget : public QPlainTextEdit
{
  Q_OBJECT

public:
    enum ConsoleMode {
        Input,
        Output
    };
    Q_ENUM(ConsoleMode)

    enum ConsoleChannel {
        StandardOutput,
        StandardError
    };
    Q_ENUM(ConsoleChannel)

  QConsoleWidget(QWidget* parent = 0);

  ~QConsoleWidget();

  ConsoleMode mode() const { return _mode; }
  void setMode(ConsoleMode m);

  // get the io buffers
  QIODevice* device() const { return (QIODevice*)_iodevice; }

  QTextCharFormat stdInFormat() const { return _stdInFormat; }
  void setStdInFormat(const QTextCharFormat& fmt)
  { _stdInFormat = fmt; }
  QTextCharFormat stdOutFormat() const { return _stdOutFormat; }
  void setStdOutFormat(const QTextCharFormat& fmt)
  { _stdOutFormat = fmt; }
  QTextCharFormat stdErrFormat() const { return _stdErrFormat; }
  void setStdErrFormat(const QTextCharFormat& fmt)
  { _stdErrFormat = fmt; }

  virtual QSize	sizeHint() const
  { return QSize(600,400); }


public slots:

  //! overridden to control which characters a user may delete
  virtual void cut();

  //! write to StandardOutput
  void writeStdOut(const QString& s);

  //! write to StandardError
  void writeStdErr(const QString& s);

signals:
  void consoleCommand(const QString& code);
  void abortEvaluation();


protected:
  //! hanle the return key press
  void handleReturnKey();

  //! write a formatted message line to the console
  void writeOutput(const QString & message, const QTextCharFormat& fmt);

  //! derived key press event
  virtual void keyPressEvent (QKeyEvent * e);

  //! Returns true if selection is in edit zone
  bool isSelectionInEditBlock();

  //! replace the command line
  void replaceCommandLine(const QString& str);

  //! get the current command line
  QString getCommandLine();

private:

  class QConsoleHistory
  {
      QStringList strings_;
      int pos_;
      QString token_;
      bool active_;
      int maxsize_;
  public:
      QConsoleHistory(void);
      ~QConsoleHistory(void);

      void add(const QString& str);

      const QString& currentValue() const
      {
          return pos_ == -1 ? token_ : strings_.at(pos_);
      }

      void activate(const QString& tk = QString());
      void deactivate() { active_ = false; }
      bool isActive() const { return active_; }

      bool move(bool dir);

      int indexOf(bool dir, int from) const;
  };

  static QConsoleHistory _history;

  ConsoleMode _mode;
    int inpos_;

  QString _currentMultiLineCode;

  QConsoleIODevice* _iodevice;

  QTextCharFormat _stdInFormat;
  QTextCharFormat _stdOutFormat;
  QTextCharFormat _stdErrFormat;

  QCompleter* _completer;

};



#endif
