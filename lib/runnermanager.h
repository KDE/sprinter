#ifndef RUNNERMANAGER
#define RUNNERMANAGER

#include <QAbstractItemModel>

class RunnerManager : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString query WRITE setQuery READ query NOTIFY queryChanged)

public:
    RunnerManager(QObject *parent = 0);
    ~RunnerManager();

    QString query() const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    // setRunners
    // listRunners

public Q_SLOTS:
    void setQuery(const QString &query);

Q_SIGNALS:
    void queryChanged(const QString &query);
    void querySessionCompleted();

private:
    class Private;
    Private * const d;
};

#endif

