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

#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QObject>
#include <QRunnable>

#include <querycontext.h>
#include <runnersessiondata.h>

class QueryMatch;
class AbstractRunner;
class QueryContext;
class RunnerSessionData;

class AbstractRunner : public QObject
{
    Q_OBJECT

public:
    AbstractRunner(QObject *parent);
    ~AbstractRunner();

    QString id() const;
    void setId(const QString &newId);

    virtual RunnerSessionData *createSessionData();

    void startMatch(RunnerSessionData *sessionData, const QueryContext &context);
    bool startExec(const QueryMatch &match);

    void setMinQueryLength(uint length);
    int minQueryLength() const;


protected:
    virtual void match(RunnerSessionData *sessionData, const QueryContext &context);
    virtual bool exec(const QueryMatch &match);

private:
    class Private;
    Private * const d;
};

class AbstractRunnerFactory : public QObject
{
    Q_OBJECT

public:
    AbstractRunnerFactory(QObject *parent = 0) : QObject(parent) {}
    virtual AbstractRunner *create(const QString &id, QObject *parent = 0)
    {
        return 0;
    }
};

#define RUNNER_FACTORY(type, id, json) \
class RunnerFactory : public AbstractRunnerFactory { \
    Q_OBJECT \
    Q_PLUGIN_METADATA(IID #id FILE #json) \
public: \
    RunnerFactory(QObject *parent = 0) : AbstractRunnerFactory(parent) {} \
    AbstractRunner *create(const QString &runnerId, QObject *parent = 0) {\
        type *r = new type(parent);\
        r->setId(runnerId);\
        return r;\
    }\
};

#endif