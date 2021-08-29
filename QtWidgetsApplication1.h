#include <QWidget>
#include<qtimer>
#include <QSqlQueryModel>
#include <QtNetwork>
#include<qtableview>
#include<qlabel>
#include<QLineEdit>
QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE
    
class QtWidgetsApplication1 : public QWidget
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = Q_NULLPTR); 
    void Start();
private:
    QString hash,title;
    QMap<QNetworkReply*,quint64> replys;
    QNetworkAccessManager manager;
    QLineEdit * picBar;
    int downQueued;
private slots:
    void downloadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    
    void picFinished();
};
