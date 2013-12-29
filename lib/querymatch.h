#ifndef QUERYMATCH
#define QUERYMATCH

#include <QString>
#include <QExplicitlySharedDataPointer>

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

    QueryMatch();
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    QueryMatch &operator=(const QueryMatch &other);

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(Type type);
    Type type() const;

    void setPrecision(Precision precision);
    Precision precision() const;

    /**
     * @arg interval the number of seconds with which to attempt to update
     * this match; 0 means "never" and is the default
     */
    void setUpdateInterval(uint interval);
    uint updateInterval() const;

    void setInternalId(const QString &id);
    QString internalId() const;

    //TODO run match, icon, actions (sub-QueryMatches?), id (QUuid?)
private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
