#include "QConsoleWidget.h"
#include "QConsoleIODevice.h"

#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QTextDocumentFragment>
#include <QTextBlock>
#include <QTextCursor>
#include <QDebug>
#include <QStringListModel>
#include <QScrollBar>
#include <QAbstractItemView>


QConsoleWidget::QConsoleHistory QConsoleWidget::_history;

//-----------------------------------------------------------------------------

QConsoleWidget::QConsoleWidget(QWidget* parent)
: QPlainTextEdit(parent), _mode(Output)
{

    _iodevice = new QConsoleIODevice(this,this);

   _stdInFormat = _stdOutFormat = _stdErrFormat = currentCharFormat();
	_stdOutFormat.setForeground(Qt::darkBlue);
	_stdErrFormat.setForeground(Qt::red);


  setTextInteractionFlags(Qt::TextEditorInteraction);
  //setContextMenuPolicy(Qt::NoContextMenu);

}

void QConsoleWidget::setMode(ConsoleMode m)
{
    if (m==_mode) return;

    if (m==Input) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        setCurrentCharFormat(_stdInFormat);
        inpos_ = cursor.position();
        _mode = Input;
    }

    if (m==Output) {
        _mode = Output;
    }

}



//-----------------------------------------------------------------------------

QConsoleWidget::~QConsoleWidget() {
}





//-----------------------------------------------------------------------------

QString QConsoleWidget::getCommandLine()
{
    if (_mode==Output) return QString();

    // select text in edit zone (from the input pos to the end)
  QTextCursor textCursor = this->textCursor();
  textCursor.movePosition(QTextCursor::End);
  textCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor,
                          textCursor.position() - inpos_);
  return textCursor.selectedText();
}

//-----------------------------------------------------------------------------

void QConsoleWidget::handleReturnKey()
{
    QString code = getCommandLine();

    // start new block
    appendPlainText(QString());
    setMode(Output);


    QTextCursor textCursor = this->textCursor();
    textCursor.movePosition(QTextCursor::End);
    setTextCursor(textCursor);

    // Update the history
    if (!code.isEmpty()) _history.add(code);

    // append the newline char and
    // send signal / update iodevice
    code += "\n";
    if (_iodevice->isOpen())
        _iodevice->consoleWidgetInput(code);
    else {
        emit consoleCommand(code);
    }

}

void QConsoleWidget::keyPressEvent(QKeyEvent* event) {

    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->key()==Qt::Key_Q) // Ctrl-Q aborts
        {
            emit abortEvaluation();
            event->accept();
            return;
        }
    }

    if (mode()!=Input) {
        event->ignore();
        return;
    }

    int key = event->key();


    if (_history.isActive())
    {
        switch (key)
        {
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (_history.move(key==Qt::Key_Up))
                //changeHistory();
                replaceCommandLine(_history.currentValue());
            else QApplication::beep();
            event->accept();
            return;
        default:
            _history.deactivate();
            break;
        }
    }

    bool        eventHandled = false;
    QTextCursor textCursor   = this->textCursor();

    switch (key) {

//    case Qt::Key_Tab:
//        handleTabCompletion();
//        eventHandled = true;
//        return;

    case Qt::Key_Escape:
        replaceCommandLine(QString());
        eventHandled = true;
        break;


    case Qt::Key_Left:

        // Moving the cursor left is limited to the position
        // of the command prompt.

        if (textCursor.position() <= inpos_) {

            QApplication::beep();
            eventHandled = true;
        }
        break;

    case Qt::Key_Up:

        // Activate the history and move to the 1st matching history item

        if (!_history.isActive()) _history.activate(getCommandLine());

        if (_history.move(key==Qt::Key_Up))
            replaceCommandLine(_history.currentValue());
        else QApplication::beep();

        eventHandled = true;
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:

        // Execute the current command

        handleReturnKey();
        eventHandled = true;
        break;

    case Qt::Key_Backspace:

        if (textCursor.hasSelection()) {

            cut();
            eventHandled = true;

        } else {

            // Intercept backspace key event to check if
            // deleting a character is allowed. It is not
            // allowed, if the user wants to delete the
            // command prompt.

            if (textCursor.position() <= inpos_) {

                QApplication::beep();
                eventHandled = true;
            }
        }
        break;

    case Qt::Key_Delete:

        if (textCursor.hasSelection()) {

            cut();
            eventHandled = true;

        } else {

            // Intercept delete key event to check if
            // cursor is after the prompt

            if (textCursor.position() < inpos_) {

                QApplication::beep();
                eventHandled = true;
            }
        }
        break;

    default:

        // Check if the key is an input character
        if (key >= Qt::Key_Space && key <= Qt::Key_division) {


            if (textCursor.hasSelection() && !isSelectionInEditBlock()) {

                // If the selection is not in the edit block
                // it should not be replaced
                eventHandled = true;

            } else {

                // check if the cursor is
                // behind the last command prompt, else inserting the
                // character is not allowed.

                 if (textCursor.position() < inpos_) {

                    QApplication::beep();
                    eventHandled = true;

                }
            }
        }
    }

    if (eventHandled) {

        event->accept();

    } else {

        QPlainTextEdit::keyPressEvent(event);

    }
}



//-----------------------------------------------------------------------------

void QConsoleWidget::cut() {
  if (isSelectionInEditBlock()) {
    QPlainTextEdit::cut();
  }
}



//-----------------------------------------------------------------------------

bool QConsoleWidget::isSelectionInEditBlock()
{
    QTextCursor textCursor = this->textCursor();
    if (!textCursor.hasSelection()) return false;

    int selectionStart    = textCursor.selectionStart();
    int selectionEnd      = textCursor.selectionEnd();

    return selectionStart >= inpos_ && selectionEnd >= inpos_;
}




//-----------------------------------------------------------------------------

void QConsoleWidget::replaceCommandLine(const QString& str) {

  // Select the text after the last command prompt ...
  QTextCursor textCursor = this->textCursor();
  textCursor.movePosition(QTextCursor::End);
  textCursor.setPosition(inpos_, QTextCursor::KeepAnchor);

  // ... and replace it with new string.
  textCursor.insertText(str,_stdInFormat);

  // move to the end of the document
  textCursor.movePosition(QTextCursor::End);
  setTextCursor(textCursor);
}



//-----------------------------------------------------------------------------

void QConsoleWidget::writeOutput(const QString & message, const QTextCharFormat& fmt)
{
    QTextCharFormat currfmt = currentCharFormat();
    QTextCursor tc = textCursor();

    if (mode()==Input)
	{
        // in Input mode output messages are inserted
        // before the edit block

        // get offset of current pos from the end
		int editpos = tc.position();
        tc.movePosition(QTextCursor::End);
		editpos = tc.position() - editpos;

        // convert the input pos as relative from the end
        inpos_ = tc.position() - inpos_;

        // insert block
		tc.movePosition(QTextCursor::StartOfBlock);
		tc.insertBlock();
		tc.movePosition(QTextCursor::PreviousBlock);

		tc.insertText(message,fmt);
		tc.movePosition(QTextCursor::End);
        // restore input pos
        inpos_ = tc.position() - inpos_;
        // restore the edit pos
		tc.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,editpos);
		setTextCursor(tc);
        setCurrentCharFormat(currfmt);
	}
	else
	{
        // in output mode messages are appended
        QTextCursor tc1 = tc;
        tc1.movePosition(QTextCursor::End);

        // check is cursor was not at the end
        // (e.g. had been moved per mouse action)
        bool needsRestore = tc1.position()!=tc.position();

        // insert text
        setTextCursor(tc1);
		textCursor().insertText(message,fmt);
        ensureCursorVisible();

        // restore cursor if needed
        if (needsRestore) setTextCursor(tc);
	}


}

void QConsoleWidget::writeStdOut(const QString& s)
{
    writeOutput(s,_stdOutFormat);
}

void QConsoleWidget::writeStdErr(const QString& s)
{
    writeOutput(s,_stdErrFormat);
}

/////////////////// QConsoleWidget::QConsoleHistory /////////////////////

#define HISTORY_FILE ".command_history.lst"

QConsoleWidget::QConsoleHistory::QConsoleHistory(void) : pos_(0), active_(false), maxsize_(10000)
{
    QFile f(HISTORY_FILE);
    if (f.open(QFile::ReadOnly)) {
        QTextStream is(&f);
        while(!is.atEnd()) add(is.readLine());
    }
}
QConsoleWidget::QConsoleHistory::~QConsoleHistory(void)
{
    QFile f(HISTORY_FILE);
    if (f.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream os(&f);
        int n = strings_.size();
        while(n>0) os << strings_.at(--n) << endl;
    }
}

void QConsoleWidget::QConsoleHistory::add(const QString& str)
{
    if (strings_.size() == maxsize_) strings_.pop_back();
    strings_.push_front(str);
    active_ = false;
}

void QConsoleWidget::QConsoleHistory::activate(const QString& tk)
{
    active_ = true;
    token_ = tk;
    pos_ = -1;
}

bool QConsoleWidget::QConsoleHistory::move(bool dir)
{
    if (active_)
    {
        int next = indexOf ( dir, pos_ );
        if (pos_!=next)
        {
            pos_=next;
            return true;
        }
        else return false;
    }
    else return false;
}

int QConsoleWidget::QConsoleHistory::indexOf(bool dir, int from) const
{
    int i = from, to = from;
    if (dir)
    {
        while(i < strings_.size()-1)
        {
            const QString& si = strings_.at(++i);
            if (si.startsWith(token_)) { return i; }
        }
    }
    else
    {
        while(i > 0)
        {
            const QString& si = strings_.at(--i);
            if (si.startsWith(token_)) { return i; }
        }
        return -1;
    }
    return to;
}

