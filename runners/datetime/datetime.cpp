/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datetime.h"

#include <QDebug>
#include <QLocale>

static const QString dateWord = QObject::tr("date");
static const QString timeWord = QObject::tr("time");

DateTimeRunner::DateTimeRunner(QObject *parent)
    : AbstractRunner(parent)
{
}

QueryMatch DateTimeRunner::createMatch(const QString &title, const QString &userData, const QString &data)
{
    QueryMatch match(this);
    match.setTitle(title);
    match.setUserData(userData);
    match.setData(data);
    match.setPrecision(QueryMatch::ExactMatch);
    match.setType(QueryMatch::InformationalType);
//     match.setIcon(KIcon(QLatin1String( "clock" )));
    return match;
}

void DateTimeRunner::populateTzList()
{
    QDateTime dt(QDateTime::currentDateTime());
    QString abbrev;
//     qDebug() << "POPULATING!";
    foreach (const QByteArray &tzId, QTimeZone::availableTimeZoneIds()) {
        qDebug() << tzId;
        QString searchableTz(tzId);
        m_tzList.insert(searchableTz.replace('_', ' '), tzId);
        QTimeZone tz(tzId);

        abbrev = tz.abbreviation(dt);
                qDebug() << abbrev;
        if (!abbrev.isEmpty()) {
            m_tzList.insert(abbrev, abbrev.toLatin1());
        }
    }
}

QDateTime DateTimeRunner::datetime(const QString &term, bool date, QString &tzName, QString &matchData)
{
    const QString tz = term.right(term.length() - (date ? dateWord.length() : timeWord.length()) - 1);

    if (tz.length() < 3) {
        return QDateTime();
    }

    if (tz.compare(QLatin1String("UTC"), Qt::CaseInsensitive) == 0) {
        matchData = (date ? dateWord : timeWord) + ":UTC";
        tzName = QLatin1String("UTC");
        QDateTime UTC(QDateTime::currentDateTime());
        UTC.setTimeSpec(Qt::UTC);
        return UTC;
    }

    if (m_tzList.isEmpty()) {
        populateTzList();
    }

    QDateTime dt;
    QHashIterator<QString, QByteArray> it(m_tzList);
    while (it.hasNext()) {
        it.next();
        if (it.key().compare(tz, Qt::CaseInsensitive) == 0) {
            matchData = (date ? dateWord : timeWord) + ":" + it.key();
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
            break;
        } else if (!dt.isValid() &&
                   it.key().contains(tz, Qt::CaseInsensitive)) {
            matchData = (date ? dateWord : timeWord) + ":" + it.key();
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
        }
    }

    return dt;
}

void DateTimeRunner::match(RunnerSessionData *sessionData, const RunnerContext &context)
{
    const QString term = context.query();
    QVector<QueryMatch> matches;

//     qDebug() << "checking" << term;
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QString date = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        matches << createMatch(date, date, dateWord);
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString matchData;
        QDateTime dt = datetime(term, true, tzName, matchData);
        if (dt.isValid()) {
            const QString date = dt.date().toString(Qt::SystemLocaleShortDate);
            matches << createMatch(QString("%2 (%1)").arg(tzName, date), date, matchData);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
        QueryMatch match = createMatch(time, time, timeWord);
        match.setUpdateInterval(1);
        matches << match;
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString matchData;
        QDateTime dt = datetime(term, false, tzName, matchData);
        if (dt.isValid()) {
            const QString time = dt.time().toString(Qt::SystemLocaleShortDate);
            QueryMatch match = createMatch(QString("%2 (%1)").arg(tzName, time), time, matchData);
            match.setUpdateInterval(1);
            matches << match;
        }
    }

    sessionData->setMatches(matches, context);
}

#include "moc_datetime.cpp"