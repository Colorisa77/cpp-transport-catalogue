#pragma once
#include "json.h"
#include <deque>

using namespace std::literals;

namespace json {

    class KeyBuilderHelper;
    class ArrayBuilderHelper;
    class DictBuilderHelper;

    class Builder {
        struct AfterDict {
            bool is_after_key_ = false;
            bool is_after_start_dict_ = false;
        };

    public:
        Builder() = default;
        ~Builder();

        KeyBuilderHelper Key(const std::string& key);
        Builder& Value(const Node& value);
        DictBuilderHelper StartDict();
        Builder& EndDict();
        ArrayBuilderHelper StartArray();
        Builder& EndArray();

        Node Build();

    private:
        json::Node root_;
        std::vector<Node*> nodes_stack_;
        int curr_node_ = -1;
        std::deque<AfterDict> stack_of_dict_info_;
        std::deque<std::string> stack_of_dict_keys_;
        std::string curr_key_;
    };

    class ValueAfterKeyBuilderHelper;
    class ValueAfterStartArrayBuilderHelper;

    class BuilderHelper {
    public:
        BuilderHelper(Builder& builder);
        ~BuilderHelper() = default;

        KeyBuilderHelper Key(const std::string& key);
        Builder& Value(const json::Node& value);
        DictBuilderHelper StartDict();
        Builder& EndDict();
        ArrayBuilderHelper StartArray();
        Builder& EndArray();
        Node Build();
    protected:
        Builder& builder_;
    };

    class ValueAfterKeyBuilderHelper : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        Builder& Value(const json::Node&) = delete;
        DictBuilderHelper StartDict() = delete;
        ArrayBuilderHelper StartArray() = delete;
        Builder& EndArray() = delete;
        Node Build() = delete;
    };

    class ValueAfterStartArrayBuilderHelper : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        ValueAfterStartArrayBuilderHelper Value(const json::Node& value);
        KeyBuilderHelper Key(const std::string&) = delete;
        Builder& EndDict() = delete;
        Node Build() = delete;

    };

    class KeyBuilderHelper : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        ValueAfterKeyBuilderHelper Value(const json::Node& value);
        KeyBuilderHelper Key(const std::string&) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
        Node Build() = delete;
    };

    class ArrayBuilderHelper : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        ValueAfterStartArrayBuilderHelper Value(const json::Node& value);
        KeyBuilderHelper Key(const std::string&) = delete;
        Builder& EndDict() = delete;
        Node Build() = delete;
    };

    class DictBuilderHelper : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        Builder& Value(const json::Node&) = delete;
        DictBuilderHelper StartDict() = delete;
        ArrayBuilderHelper StartArray() = delete;
        Builder& EndArray() = delete;
        Node Build() = delete;
    };

} //namespace json