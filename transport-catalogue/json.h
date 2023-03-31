#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value =  std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;

        Node(Array array);

        Node(bool value);

        Node(std::nullptr_t);

        Node(Dict map);

        Node(int value);

        Node(double value);

        Node(std::string value):
            value_(std::move(value)) {}

        bool operator==(const Node& rhs) const { return GetValue() == rhs.GetValue(); }

        [[nodiscard]] const Value& GetValue() const { return value_; }

        [[nodiscard]] bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }

        [[nodiscard]] bool IsBool() const { return std::holds_alternative<bool>(value_); }

        [[nodiscard]] bool IsInt() const { return std::holds_alternative<int>(value_); }

        [[nodiscard]] bool IsPureDouble() const { return std::holds_alternative<double>(value_); }

        [[nodiscard]] bool IsDouble() const { return IsInt() || IsPureDouble(); }

        [[nodiscard]] bool IsArray() const { return std::holds_alternative<Array>(value_); }

        [[nodiscard]] bool IsMap() const { return std::holds_alternative<Dict>(value_); }

        [[nodiscard]] bool IsString() const { return std::holds_alternative<std::string>(value_); }

        int AsInt() const {
            using namespace std::literals;
            if (!IsInt()) {
                throw std::logic_error("Not an int"s);
            }
            return std::get<int>(value_);
        }

        double AsDouble() const {
            using namespace std::literals;
            if (!IsDouble()) {
                throw std::logic_error("Not a double"s);
            }
            return IsPureDouble() ? std::get<double>(value_) : AsInt();
        }

        bool AsBool() const {
            using namespace std::literals;
            if (!IsBool()) {
                throw std::logic_error("Not a bool"s);
            }
            return std::get<bool>(value_);
        }

        const Array& AsArray() const {
            using namespace std::literals;
            if (!IsArray()) {
                throw std::logic_error("Not an array"s);
            }
            return std::get<Array>(value_);
        }

        const std::string& AsString() const {
            using namespace std::literals;
            if (!IsString()) {
                throw std::logic_error("Not a string"s);
            }
            return std::get<std::string>(value_);
        }

         const Dict& AsMap() const {
            using namespace std::literals;
            if (!IsMap()) {
                throw std::logic_error("Not a dict"s);
            }
            return std::get<Dict>(value_);
        }

    private:
        Value value_;
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root) :
        root_(std::move(root))
        {}

        [[nodiscard]] const Node& GetRoot() const { return root_; }

    private:
        Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    inline bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json