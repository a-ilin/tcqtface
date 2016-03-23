#ifndef CORE_H
#define CORE_H

#include <qt_windows.h>
#include <qglobal.h>

class QObject;
class QEvent;

class CorePrivate;
class InterfaceKeeper;
class TCmdParentWindow;

class Core
{
public:
  ~Core();

  static Core& i();

  static bool isExists();

  // open a new window with file specified
  HWND createWindow(const InterfaceKeeper& keeper,
                    HWND hParentWin,
                    const QString& sFilePath,
                    int iShowFlags);

  // load specified file into an existed window
  int loadFile(const InterfaceKeeper& keeper,
               HWND hParentWin,
               HWND hChildWin,
               const QString& sFilePath,
               int iShowFlags);

  void destroyWindow(const InterfaceKeeper& keeper, HWND hWin);

  int searchDialog(const InterfaceKeeper& keeper, HWND hWin,int iFindNext);

  int print(const InterfaceKeeper& keeper, HWND hWin,
            const QString& sFileToPrint, const QString& sDefPrinter,
            int iPrintFlags, RECT* pMargins);

  int sendCommand(const InterfaceKeeper& keeper, HWND hWin, int iCommand, int iParameter);

  int searchText(const InterfaceKeeper& keeper, HWND hWin,
                 const QString& sSearchString, int iSearchParameter);

private:
  Core();
  CorePrivate* d_ptr;

  Q_DISABLE_COPY(Core)
  Q_DECLARE_PRIVATE(Core)
};


#endif // CORE_H
