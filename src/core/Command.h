#pragma once

#include "core/Project.h"
#include "core/Result.h"

#include <QMetaType>
#include <QString>

namespace cutline {

// What a change touched, so views refresh only what they must.
struct ChangeHint {
    enum class Scope { Project, Sequence, Tracks };
    Scope scope = Scope::Project;
    QString sequenceId;
};

// One undoable edit. apply() validates first and leaves the project untouched when it fails;
// revert() is only ever called after a successful apply().
class Command {
public:
    virtual ~Command() = default;
    virtual QString title() const = 0;
    virtual Status apply(Project& project) = 0;
    virtual void revert(Project& project) = 0;
    virtual ChangeHint hint() const = 0;
};

} // namespace cutline

Q_DECLARE_METATYPE(cutline::ChangeHint)
