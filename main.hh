#ifndef _MIDE_MAIN_H
#define _MIDE_MAIN_H




#include <qprocess.h>
#include <vector>
#include "main_ui.hh"




using namespace std;




class mMain : public mMainWindow { 
    Q_OBJECT
    
    struct mLine {
      bool 	FromStderr;
      QString	Line;
      
      mLine(bool from_stderr,QString const &line)
        : FromStderr(from_stderr),Line(line) {
	}
      };
      
    QProcess			*Process;
    typedef vector<mLine>	mOutput;
    mOutput			Output;
    bool			ErrorSwitchHappened;

  public:
    mMain( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    
    void processOutput(int new_lines);
    void exec(QString const &cmdline,bool wait = false);

  public slots:
    void examine();
    void examine(QListViewItem *sel);
    void dataReady();
    void build();
    void stop();
    void finished();
    void about();
    void run();
  };




#endif // MFEMAIN_H
