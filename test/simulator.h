#ifndef SIMULATOR
#define SIMULATOR

#include <QObject>
#include <QStringList>

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT

public:
    Simulator(QObject * parent = 0);

Q_SIGNALS:
    void query(const QString &query);
    void done();

private Q_SLOTS:
    void generateQuery();

private:
    void setupTimer();

    QStringList m_queries;
    QStringList::const_iterator m_iterator;
    QTimer *m_timer;
    int m_pos;
};

#endif

