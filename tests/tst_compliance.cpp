#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QHostInfo>
#include <QDebug>
#include "qwebsocket.h"
#include "unittests.h"

class ComplianceTest : public QObject
{
    Q_OBJECT

public:
    ComplianceTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    /**
     * @brief Runs the autobahn tests against our implementation
     */
    void autobahnTest();

private:
    QUrl m_url;

    void runTestCases(int startNbr, int stopNbr = -1);
    void runTestCase(int nbr, int total);
};

ComplianceTest::ComplianceTest() :
    m_url("ws://localhost:9001")
{
}

void ComplianceTest::initTestCase()
{
}

void ComplianceTest::cleanupTestCase()
{
}

void ComplianceTest::init()
{
}

void ComplianceTest::cleanup()
{
}

void ComplianceTest::runTestCase(int nbr, int total)
{
    if (nbr == total)
    {
        return;
    }
    QWebSocket *pWebSocket = new QWebSocket;
    QSignalSpy spy(pWebSocket, SIGNAL(disconnected()));

    //next for every case, connect to url
    //ws://ipaddress:port/runCase?case=<number>&agent=<agentname>
    //where agent name will be QWebSocket
    QObject::connect(pWebSocket, &QWebSocket::textMessageReceived, [=](QString message) {
        pWebSocket->write(message);
    });
    QObject::connect(pWebSocket, &QWebSocket::binaryMessageReceived, [=](QByteArray message) {
        pWebSocket->write(message);
    });

    qDebug() << "Executing test" << (nbr + 1) << "/" << total;
    QUrl url = m_url;
    url.setPath("/runCase?");
    QUrlQuery query;
    query.addQueryItem("case", QString::number(nbr + 1));
    query.addQueryItem("agent", "QWebSockets/0.9");
    url.setQuery(query);
    pWebSocket->open(url);
    spy.wait(60000);
    pWebSocket->close();
    delete pWebSocket;
    pWebSocket = 0;
    runTestCase(nbr + 1, total);
}

void ComplianceTest::runTestCases(int startNbr, int stopNbr)
{
    runTestCase(startNbr, stopNbr);
}

void ComplianceTest::autobahnTest()
{
    //connect to autobahn server at url ws://ipaddress:port/getCaseCount
    QWebSocket *pWebSocket = new QWebSocket;
    QUrl url = m_url;
    int numberOfTestCases = 0;
    QSignalSpy spy(pWebSocket, SIGNAL(disconnected()));
    QObject::connect(pWebSocket, &QWebSocket::textMessageReceived, [&](QString message) {
        numberOfTestCases = message.toInt();
    });

    url.setPath("/getCaseCount");
    pWebSocket->open(url);
    spy.wait(60000);
    QVERIFY(numberOfTestCases > 0);

    QObject::disconnect(pWebSocket, &QWebSocket::textMessageReceived, 0, 0);

    runTestCases(0, numberOfTestCases);

    url.setPath("/updateReports?");
    QUrlQuery query;
    query.addQueryItem("agent", "QWebSockets");
    url.setQuery(query);
    pWebSocket->open(url);
    spy.wait(60000);
    delete pWebSocket;
    pWebSocket = 0;
}

//DECLARE_TEST(ComplianceTest)

#include "tst_compliance.moc"

