#pragma once

#include <QObject>
#include <QStringList>

namespace cutline {

class AppSettings;

// Newest first, at most 10, no duplicates; files that no longer exist drop off when the list is read.
class RecentProjects : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList paths READ paths NOTIFY changed)

public:
    explicit RecentProjects(AppSettings& settings, QObject* parent = nullptr);

    void add(const QString& path);
    QStringList paths() const;
    Q_INVOKABLE QString displayName(const QString& path) const;

signals:
    void changed();

private:
    void store(const QStringList& paths) const;
    AppSettings& m_settings;
};

} // namespace cutline
