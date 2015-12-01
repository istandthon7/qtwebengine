
#include <QtTest/QtTest>

#include <QPainter>
#include <QPdfDocument>
#include <QPrinter>
#include <QTemporaryFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class tst_QPdfDocument: public QObject
{
    Q_OBJECT
public:

private slots:
    void pageCount();
    void loadFromIODevice();
    void loadAsync();
    void password();
    void close();
    void loadAfterClose();
    void closeOnDestroy();
    void passwordClearedOnClose();
    void metaData();
};

struct TemporaryPdf: public QTemporaryFile
{
    TemporaryPdf();
    QPageLayout pageLayout;
};


TemporaryPdf::TemporaryPdf()
{
    open();
    pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF());

    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName());
        printer.setPageLayout(pageLayout);

        {
            QPainter painter(&printer);
            painter.drawText(100, 100, QStringLiteral("Hello Page 1"));
            printer.newPage();
            painter.drawText(100, 100, QStringLiteral("Hello Page 2"));
        }
    }

    seek(0);
}

void tst_QPdfDocument::pageCount()
{
    TemporaryPdf tempPdf;

    QPdfDocument doc;
    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(tempPdf.fileName()), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);

    QCOMPARE(doc.pageSize(0).toSize(), tempPdf.pageLayout.fullRectPoints().size());
}

void tst_QPdfDocument::loadFromIODevice()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;
    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));
    doc.load(&tempPdf);
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::loadAsync()
{
    TemporaryPdf tempPdf;

    QNetworkAccessManager nam;

    QUrl url = QUrl::fromLocalFile(tempPdf.fileName());
    QScopedPointer<QNetworkReply> reply(nam.get(QNetworkRequest(url)));

    QPdfDocument doc;
    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));

    doc.load(reply.data());

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::password()
{
    QPdfDocument doc;
    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    QCOMPARE(doc.pageCount(), 0);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    QCOMPARE(passwordChangedSpy.count(), 0);
    doc.setPassword(QStringLiteral("WrongPassword"));
    QCOMPARE(passwordChangedSpy.count(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::IncorrectPasswordError);
    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.count(), 2);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 1);
}

void tst_QPdfDocument::close()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy aboutToBeClosedSpy(&doc, SIGNAL(aboutToBeClosed()));

    doc.load(&tempPdf);
    QCOMPARE(aboutToBeClosedSpy.count(), 0);
    doc.close();
    QCOMPARE(aboutToBeClosedSpy.count(), 1);
    QCOMPARE(doc.pageCount(), 0);
}

void tst_QPdfDocument::loadAfterClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy aboutToBeClosedSpy(&doc, SIGNAL(aboutToBeClosed()));

    doc.load(&tempPdf);
    QCOMPARE(aboutToBeClosedSpy.count(), 0);
    doc.close();
    QCOMPARE(aboutToBeClosedSpy.count(), 1);

    QSignalSpy startedSpy(&doc, SIGNAL(documentLoadStarted()));
    QSignalSpy finishedSpy(&doc, SIGNAL(documentLoadFinished()));
    doc.load(&tempPdf);
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(doc.error(), QPdfDocument::NoError);
    QCOMPARE(doc.pageCount(), 2);
}

void tst_QPdfDocument::closeOnDestroy()
{
    TemporaryPdf tempPdf;

    // deleting an open document should automatically close it
    {
        QPdfDocument *doc = new QPdfDocument;

        QSignalSpy aboutToBeClosedSpy(doc, SIGNAL(aboutToBeClosed()));
        doc->load(&tempPdf);

        delete doc;

        QCOMPARE(aboutToBeClosedSpy.count(), 1);
    }

    // deleting a closed document should not emit any signal
    {
        QPdfDocument *doc = new QPdfDocument;
        doc->load(&tempPdf);
        doc->close();

        QSignalSpy aboutToBeClosedSpy(doc, SIGNAL(aboutToBeClosed()));

        delete doc;

        QCOMPARE(aboutToBeClosedSpy.count(), 0);
    }
}

void tst_QPdfDocument::passwordClearedOnClose()
{
    TemporaryPdf tempPdf;
    QPdfDocument doc;

    QSignalSpy passwordChangedSpy(&doc, SIGNAL(passwordChanged()));

    doc.setPassword(QStringLiteral("Qt"));
    QCOMPARE(passwordChangedSpy.count(), 1);
    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.protected.pdf")), QPdfDocument::NoError);
    passwordChangedSpy.clear();

    doc.close(); // password is cleared on close
    QCOMPARE(passwordChangedSpy.count(), 1);
    passwordChangedSpy.clear();

    doc.load(&tempPdf);
    doc.close(); // signal is not emitted if password didn't change
    QCOMPARE(passwordChangedSpy.count(), 0);
}

void tst_QPdfDocument::metaData()
{
    QPdfDocument doc;

    // a closed document does not return any meta data
    QCOMPARE(doc.metaData(QPdfDocument::Title).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Subject).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Author).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Keywords).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Producer).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::Creator).toString(), QString());
    QCOMPARE(doc.metaData(QPdfDocument::CreationDate).toDateTime(), QDateTime());
    QCOMPARE(doc.metaData(QPdfDocument::ModificationDate).toDateTime(), QDateTime());

    QCOMPARE(doc.load(QFINDTESTDATA("pdf-sample.metadata.pdf")), QPdfDocument::NoError);

    // check for proper meta data from sample document
    QCOMPARE(doc.metaData(QPdfDocument::Title).toString(), QString::fromLatin1("Qt PDF Unit Test Document"));
    QCOMPARE(doc.metaData(QPdfDocument::Subject).toString(), QString::fromLatin1("A test for meta data access"));
    QCOMPARE(doc.metaData(QPdfDocument::Author).toString(), QString::fromLatin1("John Doe"));
    QCOMPARE(doc.metaData(QPdfDocument::Keywords).toString(), QString::fromLatin1("meta data keywords"));
    QCOMPARE(doc.metaData(QPdfDocument::Producer).toString(), QString::fromLatin1("LibreOffice 5.1"));
    QCOMPARE(doc.metaData(QPdfDocument::Creator).toString(), QString::fromLatin1("Writer"));
    QCOMPARE(doc.metaData(QPdfDocument::CreationDate).toDateTime(), QDateTime(QDate(2016, 8, 7), QTime(7, 3, 6), Qt::UTC));
    QCOMPARE(doc.metaData(QPdfDocument::ModificationDate).toDateTime(), QDateTime(QDate(2016, 8, 8), QTime(8, 3, 6), Qt::UTC));
}

QTEST_MAIN(tst_QPdfDocument)

#include "tst_qpdfdocument.moc"

