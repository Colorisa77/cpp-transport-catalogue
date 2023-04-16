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
    using Value =  std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public Value {
    public:
        using Value::Value;

        
        bool operator==(const Node& rhs) const;

        [[nodiscard]] const Value& GetValue() const;
        [[nodiscard]] bool IsNull() const;
        [[nodiscard]] bool IsBool() const;
        [[nodiscard]] bool IsInt() const;
        [[nodiscard]] bool IsPureDouble() const;
        [[nodiscard]] bool IsDouble() const;
        [[nodiscard]] bool IsArray() const;
        [[nodiscard]] bool IsDict() const;
        [[nodiscard]] bool IsString() const;

        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const std::string& AsString() const;
        const Dict& AsDict() const;
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    class Document {
    public:
        explicit Document(Node root);

        [[nodiscard]] const Node& GetRoot() const;

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