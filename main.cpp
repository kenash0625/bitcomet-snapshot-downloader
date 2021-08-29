#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>
/*sync torrents from bitcomet db
user can select torrent view snapshot
delete torrents viewed
*/
#include<QTimer>
#include<QPushButton>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const char* p = "dads";
    QtWidgetsApplication1 w;
    w.show();
    w.Start();
    return a.exec();
}
