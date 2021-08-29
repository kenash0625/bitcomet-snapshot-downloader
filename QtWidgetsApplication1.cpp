#include "QtWidgetsApplication1.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include<QDockWidget>
#include<QDateTime>
#include<algorithm>
#include<QVBoxLayout>

QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
    : QWidget(parent)
{   
    QVBoxLayout* layout = new QVBoxLayout(this);
    picBar = new QLineEdit(this);
    layout->addWidget(picBar);
    setLayout(layout);
}

void QtWidgetsApplication1::Start()
{
    QFile file(QCoreApplication::applicationDirPath() + "/in.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString strmaxdt, strmindt;
    int maxsize, minsize;
    QTextStream in(&file);
    in >> strmaxdt >> strmindt >> maxsize >> minsize;
    QDateTime maxdt(QDate::fromString(strmaxdt,"yyyy/MM/dd"), QTime(0, 0, 0)), mindt(QDate::fromString(strmindt,"yyyy/MM/dd"), QTime(0, 0, 0));

    QFile dbfile(QCoreApplication::applicationDirPath() +"/peer_shares.db");
    if (!dbfile.exists()) return;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "srcDb");
    db.setDatabaseName(QCoreApplication::applicationDirPath() +"/peer_shares.db");
    db.setConnectOptions("QSQLITE_OPEN_READONLY=TRUE");
    if (!db.open()) {
        return;
    }
    hash.clear();
    replys.clear();
    title.clear();
    picBar->setText("");
    downQueued = 0;
    QSqlQuery query(db);
    QString strq = "SELECT hash,title,size,createtime from PeerShares where category='video' and size>" +
        QString::number((qulonglong)minsize * (qulonglong)1e9) + " and size<" + QString::number((qulonglong)maxsize * (qulonglong)1e9)
        + " and createtime>" + QString::number(mindt.toSecsSinceEpoch()) + " and createtime<" + QString::number(maxdt.toSecsSinceEpoch());
    bool b= query.exec(strq);
    QString s= query.lastError().text();
    while (query.next()) {
        hash = query.value(0).toString();
        title = query.value(1).toString();
        auto sizegb = query.value(2).toString();
        auto createtime = query.value(3).toULongLong();
        QFile file(QCoreApplication::applicationDirPath() + "/snapshot/" + hash + "/fin.txt");
        if (file.exists()) continue;
        QDir dir;
        dir.mkpath(QCoreApplication::applicationDirPath() + "/snapshot/"+hash);
        QString str = QString("http://fileshot.net/get/torrent/") + hash + "/" + sizegb + "/en_us/1.73/";
        QUrl url(str);
        QNetworkRequest request(url);
        QNetworkReply* currentDownload = manager.get(request);
        replys[currentDownload]=0;
        connect(currentDownload, &QNetworkReply::downloadProgress, this, &QtWidgetsApplication1::downloadProgress);
        connect(currentDownload, &QNetworkReply::finished, this, &QtWidgetsApplication1::downloadFinished);
        break;
    }
    db.close();
}

void QtWidgetsApplication1::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && replys.contains(reply)) {
        replys[reply] = bytesReceived / 1024;
        quint64 sum(0);
        for (auto a : replys) { sum += a; }
        picBar->setText(QString::number(sum));
    }
}


void QtWidgetsApplication1::downloadFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString strfind("//image.fileshot.net/original"), strpage(reply->readAll());

    QFile pageFile(QCoreApplication::applicationDirPath() + "/snapshot/" + hash + "/page.txt");
    if (!pageFile.open(QIODevice::ReadWrite))
    {

    }
    pageFile.write(strpage.toUtf8());
    int findcnt(0),idx=replys.size();
    for (size_t off = 0; (off = strpage.indexOf(strfind, off)) != -1; off += 1, findcnt++)
    {
        QString url = "http:" + QStringRef(&strpage, off, strpage.indexOf('"', off) - off);
        pageFile.write(QString("\n" + url).toUtf8());
        QNetworkRequest request(url);
        QNetworkReply* currentDownload = manager.get(request);
        connect(currentDownload, &QNetworkReply::downloadProgress, this, &QtWidgetsApplication1::downloadProgress);
        connect(currentDownload, &QNetworkReply::finished, this, &QtWidgetsApplication1::picFinished);
        replys[currentDownload] = 0;
    }   
    if (findcnt == 0)
    {
        pageFile.write("\nnot available");
        pageFile.close();
        reply->deleteLater();
        QFile file(QCoreApplication::applicationDirPath() + "/snapshot/" + hash + "/fin.txt");
        file.open(QIODevice::ReadWrite);
        file.write(title.toUtf8());
        file.close();
        Start();
    }
    downQueued = 1;
}
void QtWidgetsApplication1::picFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    //QPixmap px(reply->readAll());
    QString str = reply->request().url().toString();
    str = str.mid(str.indexOf("//image.fileshot.net/original") + 31);
    str = QCoreApplication::applicationDirPath() + "/snapshot/" + hash+"/"+QString::number((quint64)reply)+".jpg";
    QFile files(str);
    files.open(QIODevice::WriteOnly);
    files.write(reply->readAll());
    files.close();
    //bool b=px.save(str, "JPG", 100);

    if (!downQueued) return;
    for (auto& a : replys.keys())
    {
        if (!a->isFinished()) return;
    }
    for (auto& a : replys.keys())
    {
        a->deleteLater();
    }
    QFile file(QCoreApplication::applicationDirPath() + "/snapshot/" + hash + "/fin.txt");
    file.open(QIODevice::ReadWrite);
    file.write(title.toUtf8());
    file.close();
    Start();
}
