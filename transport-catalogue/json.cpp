#include "json.h"

using namespace std;

namespace json {

    Node::Node(Array array) :
    value_(std::move(array)) {}

    Node::Node(bool value) :
            value_(value) {}

    Node::Node(std::nullptr_t) :
    Node() {}

    Node::Node(Dict map) :
    value_(std::move(map)) {}

    Node::Node(int value) :
            value_(value) {}

    Node::Node(double value) :
            value_(value) {}

//    Node::Node(std::string value) :
//    value_(std::move(value)) {}

    namespace {

        Node LoadNode(istream& input);

        std::string LoadLiteral(std::istream& input) {

            std::string str;
            while (std::isalpha(input.peek())) {
                str.push_back(static_cast<char>(input.get()));
            }

            return str;
        }

        Node LoadArray(std::istream& input) {

            std::vector<Node> result;
            for (char current_char; input >> current_char && current_char != ']';) {
                if (current_char != ',') {
                    input.putback(current_char);
                }
                result.push_back(LoadNode(input));
            }

            if (!input) {
                throw ParsingError("Array parsing error"s);
            }

            return Node(result);
        }

        Node LoadNull(std::istream& input) {
            if (auto literal = LoadLiteral(input); literal == "null"sv) {
                return Node{ nullptr };
            } else {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadBool(std::istream& input) {
            const auto str = LoadLiteral(input);

            if (str == "true"sv) {
                return Node{ true };
            } else if (str == "false"sv) {
                return Node{ false };
            } else {
                throw ParsingError("Failed to parse '"s + str + "' as bool"s);
            }
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }

            if (input.peek() == '0') {
                read_char();
            } else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int current_char = input.peek(); current_char == 'e' || current_char == 'E') {
                read_char();
                if (current_char = input.peek(); current_char == '+' || current_char == '-') {
                    read_char();
                }

                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                    }
                }

                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();

            std::string str;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char current_char = *it;
                if (current_char == '"') {
                    ++it;
                    break;
                } else if (current_char == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                        case 'n':
                            str.push_back('\n');
                            break;
                        case 't':
                            str.push_back('\t');
                            break;
                        case 'r':
                            str.push_back('\r');
                            break;
                        case '"':
                            str.push_back('"');
                            break;
                        case '\\':
                            str.push_back('\\');
                            break;
                        default:
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (current_char == '\n' || current_char == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    str.push_back(current_char);
                }
                ++it;
            }

            return Node(str);
        }

        Node LoadDict(std::istream& input) {

            Dict dict;
            for (char current_char; input >> current_char && current_char != '}';) {
                if (current_char == '"') {
                    std::string key = LoadString(input).AsString();

                    if (input >> current_char && current_char == ':') {
                        if (dict.find(key) != dict.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }

                        dict.emplace(std::move(key), LoadNode(input));
                    } else {
                        throw ParsingError(": is expected but '"s + current_char + "' has been found"s);
                    }
                } else if (current_char != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + current_char + "' has been found"s);
                }
            }

            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }

            return Node(dict);
        }


        Node LoadNode(std::istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError(""s);
            }
            switch (c) {
                case '[':
                    return LoadArray(input);
                case '{':
                    return LoadDict(input);
                case '"':
                    return LoadString(input);
                case 't': case 'f':
                    input.putback(c);
                    return LoadBool(input);
                case 'n':
                    input.putback(c);
                    return LoadNull(input);
                default:
                    input.putback(c);
                    return LoadNumber(input);
            }
        }

    }  // namespace

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        [[nodiscard]] PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    void PrintNode(const Node& node, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char current_char : value) {
            switch (current_char) {
                case '\r':
                    out << R"(\r)";
                    break;
                case '\n':
                    out << R"(\n)";
                    break;
                case '"':
                    out << R"(\")";
                    break;
                case '\\':
                    out << R"(\\)";
                    break;
                default:
                    out.put(current_char);
                    break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
        PrintString(value, ctx.out);
    }

    void PrintValue(const std::nullptr_t&, const PrintContext& ctx) {
        ctx.out << "null"s;
    }


    void PrintValue(bool value, const PrintContext& ctx) {
        ctx.out << std::boolalpha << value;
    }

    [[maybe_unused]] void PrintValue(Array nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;

        out << "[\n"sv;

        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes) {
            if (first) {
                first = false;
            } else {
                out << ",\n"sv;
            }

            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }

        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    [[maybe_unused]] void PrintValue(Dict nodes, const PrintContext& ctx) {
        std::ostream& out = ctx.out;

        out << "{\n"sv;

        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            } else {
                out << ",\n"sv;
            }

            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }

        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx) {
        std::visit(
                [&ctx](const auto& value) {
                    PrintValue(value, ctx);
                },
                node.GetValue());
    }



    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json