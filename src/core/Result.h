#pragma once

#include <QString>

#include <utility>
#include <variant>

namespace cutline {

struct Error {
    enum class Code { Io, Corrupt, MissingField, WrongType, InvalidValue, NewerVersion, NotACutlineProject, Validation };
    Code code = Code::Validation;
    QString message;
};

// Either a value or an Error. Move-only values (std::unique_ptr) are supported.
template <class T>
class Result {
public:
    Result(T value) : v_(std::move(value)) {}
    Result(Error error) : v_(std::move(error)) {}

    bool ok() const { return std::holds_alternative<T>(v_); }
    const T& value() const& { return std::get<T>(v_); }
    T&& value() && { return std::get<T>(std::move(v_)); }
    const Error& error() const { return std::get<Error>(v_); }

private:
    std::variant<T, Error> v_;
};

struct Status {
    bool ok = true;
    QString message;

    static Status success() { return {}; }
    static Status failure(QString message) { return {false, std::move(message)}; }
    explicit operator bool() const { return ok; }
};

} // namespace cutline
