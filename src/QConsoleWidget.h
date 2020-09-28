#ifndef _QCONSOLEWIDGET_H_
#define _QCONSOLEWIDGET_H_

#include <QPlainTextEdit>
#include <QTextStream>
#include <QCompleter>

//class QCompleter;
class QConsoleIODevice;
class QConsoleWidgetCompleter;

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
        StandardInput,
        StandardOutput,
        StandardError,
        nConsoleChannels
    };
    Q_ENUM(ConsoleChannel)

  QConsoleWidget(QWidget* parent = 0);

  ~QConsoleWidget();

  ConsoleMode mode() const { return _mode; }
  void setMode(ConsoleMode m);

  // get the io buffers
  QIODevice* device() const { return (QIODevice*)_iodevice; }

  QTextCharFormat channelCharFormat(ConsoleChannel ch) const
  { return chanFormat_[ch]; }
  void setChannelCharFormat(ConsoleChannel ch, const QTextCharFormat& fmt)
  { chanFormat_[ch] = fmt; }

  const QStringList& completionTriggers() const
  { return completion_triggers_; }
  void setCompletionTriggers(const QStringList& l)
  { completion_triggers_ = l; }

  virtual QSize	sizeHint() const
  { return QSize(600,400); }

  //! write a formatted message to the console
  void write(const QString & message, const QTextCharFormat& fmt);

  static const QStringList& history()
  { return _history.strings_; }

  void setCompleter(QConsoleWidgetCompleter* c);

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

    void handleTabKey();

    void updateCompleter();

    void checkCompletionTriggers();

  //! derived key press event
  void keyPressEvent (QKeyEvent * e) override;

  //! Returns true if selection is in edit zone
  bool isSelectionInEditZone();

  //! Returns true if cursor is in edit zone
  bool isCursorInEditZone();

  //! replace the command line
  void replaceCommandLine(const QString& str);

  //! get the current command line
  QString getCommandLine();

protected slots:
  void insertCompletion(const QString& completion);

private:

  struct History
  {
      QStringList strings_;
      int pos_;
      QString token_;
      bool active_;
      int maxsize_;
      History(void);
      ~History(void);
      void add(const QString& str);
      const QString& currentValue() const
      { return pos_ == -1 ? token_ : strings_.at(pos_); }
      void activate(const QString& tk = QString());
      void deactivate() { active_ = false; }
      bool isActive() const { return active_; }
      bool move(bool dir);
      int indexOf(bool dir, int from) const;
  };

  static History _history;

  ConsoleMode _mode;
    int inpos_, completion_pos_;
    QStringList completion_triggers_;

  QString _currentMultiLineCode;

  QConsoleIODevice* _iodevice;

  QTextCharFormat chanFormat_[nConsoleChannels];

  QConsoleWidgetCompleter* completer_;

};

QTextStream &waitForInput(QTextStream &s);
QTextStream &inputMode(QTextStream &s);
QTextStream &outChannel(QTextStream &s);
QTextStream &errChannel(QTextStream &s);

class QConsoleWidgetCompleter : public QCompleter
{
public:
  /**
  * Update the completion model given a string.  The given string
  * is the current console text between the cursor and the start of
  * the line.
  *
  * Return the completion count
  */
  virtual int updateCompletionModel(const QString& str) = 0;
};



#endif
