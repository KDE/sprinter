#ifndef QUERYMATCH
#define QUERYMATCH

#include <QString>
#include <QSharedDataPointer>

class AbstractRunner;

class QueryMatch
{
public:
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    void setText(const QString &text);
    QString text() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif
