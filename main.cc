#include "main.hh"
#include "about_ui.hh"
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qregexp.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <cstdlib>
#include <unistd.h>
#include <qtimer.h>
#include <sys/wait.h>
#include <cerrno>
#include <qtabwidget.h>
#include <qlabel.h>




#define BACKEND_PROGRAM		"mide-backend"




class mMessage : public QListViewItem {
  private:
    int MessageNumber;

  public:
    mMessage(QListView *lv,QString const &file,QString const &line,QString const &message,int msg_number)
      : QListViewItem(lv,file,line,message),MessageNumber(msg_number) {
      setOpen(true);
      }
    mMessage(QListViewItem *li,QString const &file,QString const &line,QString const &message,int msg_number)
      : QListViewItem(li,file,line,message),MessageNumber(msg_number) {
      setOpen(true);
      }
    QString key(int col,bool ascending) const {
      QString result;
      result.sprintf("%05i",MessageNumber);
      return result;
      }
    };
    



mMain::mMain( QWidget* parent,  const char* name, WFlags fl )
: mMainWindow( parent, name, fl ),Process(NULL) {
  connect(btnBuild,SIGNAL(clicked()),this,SLOT(build()));
  connect(btnStop,SIGNAL(clicked()),this,SLOT(stop()));
  connect(btnExamine,SIGNAL(clicked()),this,SLOT(examine()));
  connect(btnAbout,SIGNAL(clicked()),this,SLOT(about()));
  connect(btnRun,SIGNAL(clicked()),this,SLOT(run()));

  connect(listErrors,SIGNAL(doubleClicked(QListViewItem *)),this,SLOT(examine(QListViewItem *)));
  connect(listErrors,SIGNAL(returnPressed(QListViewItem *)),this,SLOT(examine(QListViewItem *)));
  listErrors->setColumnWidthMode(0,QListView::Manual);
  listErrors->setColumnWidthMode(1,QListView::Manual);
  listErrors->setColumnWidthMode(2,QListView::Maximum);

  connect(listWarnings,SIGNAL(doubleClicked(QListViewItem *)),this,SLOT(examine(QListViewItem *)));
  connect(listWarnings,SIGNAL(returnPressed(QListViewItem *)),this,SLOT(examine(QListViewItem *)));
  listWarnings->setColumnWidthMode(0,QListView::Manual);
  listWarnings->setColumnWidthMode(1,QListView::Manual);
  listWarnings->setColumnWidthMode(2,QListView::Maximum);
  }




void mMain::processOutput(int new_lines) {
  textOutput->setUpdatesEnabled(false);
  QString all;
  mOutput::iterator first = Output.begin(),last = Output.end();
  while (first != last) {
    if (first->FromStderr)
      all += "<font color=\"red\">" + first->Line + "</font><br>";
    else
      all += first->Line + "<br>";
    first++;
    }
  
  textOutput->setText(all);
  textOutput->scrollToBottom();
  textOutput->setUpdatesEnabled(true);
  
  listErrors->setUpdatesEnabled(false);
  listErrors->clear();
  listWarnings->setUpdatesEnabled(false);
  listWarnings->clear();
  first = Output.begin(),last = Output.end();

  int number_error = 0,number_warning = 0;
  QString lastfile_error,lastfile_warning;
  QListViewItem *lastitem_error = NULL,*lastitem_warning = NULL;

  QRegExp gcc_warning_file_line("^([^: ]*)\\:([^: ]*)\\: [Ww]arning\\:(.*)$");
  QRegExp gcc_file_line("^([^: ]*)\\:([^: ]*)\\:(.*)$");
  QRegExp gcc_file("^([^: ]*)\\:(.*)$");

  while (first != last) {
    if (!first->FromStderr) {
      first++;
      continue;
      }
    QString file,line,message;
    bool is_warning = false;
    bool is_error = false;
    
    if (gcc_warning_file_line.search(first->Line) >= 0) {
      file = gcc_warning_file_line.capturedTexts()[1];
      line = gcc_warning_file_line.capturedTexts()[2];
      message = gcc_warning_file_line.capturedTexts()[3];
      is_warning = true;
      }
    else if (gcc_file_line.search(first->Line) >= 0) {
      file = gcc_file_line.capturedTexts()[1];
      line = gcc_file_line.capturedTexts()[2];
      message = gcc_file_line.capturedTexts()[3];
      is_error = true;
      }
/*    else if (gcc_file.search(first->Line) >= 0) {
      file = gcc_file.capturedTexts()[1];
      message = gcc_file.capturedTexts()[2];
      matched = true;
      iserror = true;
      }*/
    
    if (is_error) {
      if (!ErrorSwitchHappened) {
	tabResults->showPage(tabResults->page(1));
	ErrorSwitchHappened = true;
	listErrors->setFocus();
	}
      QListViewItem *item;

      if (lastitem_error && lastfile_error == file)
	item = new mMessage(lastitem_error,file,line,message,number_error);
      else {
	item = new mMessage(listErrors,file,line,message,number_error);
	lastfile_error = file;
	lastitem_error = item;
	}
      number_error++;
      }
    if (is_warning) {
      QListViewItem *item;

      if (lastitem_warning && lastfile_warning == file)
	item = new mMessage(lastitem_warning,file,line,message,number_warning);
      else {
	item = new mMessage(listWarnings,file,line,message,number_warning);
	lastfile_warning = file;
	lastitem_warning = item;
	}
      number_warning++;
      }
    first++;
    }
  listErrors->setUpdatesEnabled(true);
  listErrors->triggerUpdate();
  listWarnings->setUpdatesEnabled(true);
  listWarnings->triggerUpdate();
  }




void mMain::exec(QString const &cmdline,bool wait) {
  int status;
  while (waitpid(WAIT_ANY,&status,WNOHANG) > 0);
  
  QApplication::flushX();
  pid_t pid;

  if ((pid = fork()) == 0) {
    execl("/bin/sh","/bin/sh","-c",cmdline.latin1(),NULL);
    exit(-1);
    }
  }




void mMain::examine() {
  if (tabResults->currentPage() == tabResults->page(1))
    examine(listErrors->selectedItem());
  if (tabResults->currentPage() == tabResults->page(2))
    examine(listWarnings->selectedItem());
  }




void mMain::examine(QListViewItem *sel) {
  if (sel == NULL) return;
  
  if (sel->text(1) == QString::null) {
    QString command(BACKEND_PROGRAM " open \"%1\" \"%2\"");
    command = command.arg(cmbBackend->currentText());
    command = command.arg(sel->text(0));
    exec(command);
    }
  else {
    QString command(BACKEND_PROGRAM " position \"%1\" \"%2\" \"%3\"");
    command = command.arg(cmbBackend->currentText());
    command = command.arg(sel->text(0));
    command = command.arg(sel->text(1));
    exec(command);
    }

  QLabel *label = new QLabel(sel->text(2),NULL);
  QTimer::singleShot(5000,label,SLOT(close()));
  QPoint pt(0,0);
  label->move(pt);
  label->raise();
  label->show();
  }




void mMain::dataReady() { 
  int new_lines = 0;
  while (Process->canReadLineStdout()) {
    Output.push_back(mLine(false, Process->readLineStdout()));
    new_lines++;
    }
  while (Process->canReadLineStderr()) {
    Output.push_back(mLine(true,Process->readLineStderr()));
    new_lines++;
    }
  processOutput(new_lines);
  }




void mMain::build() {
  Output.clear();
  Output.push_back(mLine(false,"<b>*** asking editor to save files</b> "));
  processOutput(1);
  
  tabResults->showPage(tabResults->page(0));
  ErrorSwitchHappened = false;

  QString command(BACKEND_PROGRAM " save \"%1\"");
  command = command.arg(cmbBackend->currentText());
  exec(command,true);
  
  btnBuild->setDisabled(true);
  btnStop->setEnabled(true);
  
  Process = new QProcess(this);
  connect(Process,SIGNAL(processExited()),this,SLOT(finished()));
  connect(Process,SIGNAL(readyReadStdout()),this,SLOT(dataReady()));
  connect(Process,SIGNAL(readyReadStderr()),this,SLOT(dataReady()));
  Process->addArgument("/bin/sh");
  Process->addArgument("-c");
  Process->addArgument(iptCommand->text().latin1());
  Process->start();
  }




void mMain::stop() {
  btnBuild->setEnabled(true);
  btnStop->setDisabled(true);
  
  Process->tryTerminate();    
  QTimer::singleShot(5000,Process,SLOT(kill()));

  Output.push_back(mLine(false,"<b>*** terminated<b>"));
  processOutput(1);

  delete Process;
  Process = NULL;
  }




void mMain::finished() {
  btnBuild->setEnabled(true);
  btnStop->setDisabled(true);

  if (Process->exitStatus() == 0) {
    Output.push_back(mLine(false,"<b>*** completed successfully</b>"));
    if (chkAutoRun->isChecked()) run();
    }
  else
    Output.push_back(mLine(false,"<b>*** ERROR<b>"));
  processOutput(1);
  QApplication::beep();
  
  delete Process;
  Process = NULL;
  }




void mMain::run() {
  if (iptRunCommand->text().length() > 0) {
    Output.push_back(mLine(false,"<b>*** running</b> "+iptRunCommand->text()));
    processOutput(1);
    exec(iptRunCommand->text());
    }
  }




void mMain::about() {
  QDialog *dlg = new mAbout(this);
  dlg->show();
  }




int main(int argc,char **argv) {
  QApplication app(argc,argv);
  mMain mwin;
  mwin.show();
  app.setMainWidget(&mwin);

  return app.exec();
  }
