#ifndef QUERYMATCH
#define QUERYMATCH

#include <QString>
#include <QSharedDataPointer>

class AbstractRunner;

class QueryMatch
{
public:
    enum Type {
        UnknownType = 0,
        InformationalType,
        FileType,
        ExecutableType,
        NetworkLocationType,
        ContactType,
        EventType,
        BookmarkType,
        DesktopShellType,
        HardwareType,
        AppActionType,
        AppSessionType,
        LocationType,
        LanguageType
    };

    enum Precision {
        UnrelatedMatch = 0,
        FuzzyMatch,
        CloseMatch,
        ExactMatch
    };

    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(Type type);
    Type type() const;

    void setPrecision(Precision precision);
    Precision precision() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif
