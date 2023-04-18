#pragma once
#include "json.h"
#include <deque>

using namespace std::literals;

namespace json {

    class KeyBuilder;
    class ArrayBuilder;
    class DictBuilder;

    class Builder {
        struct AfterDict {
            bool is_after_key_ = false;
            bool is_after_start_dict_ = false;
        };

    public:
        Builder() = default;
        ~Builder();

        KeyBuilder Key(const std::string& key);
        Builder& Value(const Node& value);
        DictBuilder StartDict();
        Builder& EndDict();
        ArrayBuilder StartArray();
        Builder& EndArray();

        Node Build();

    private:
        json::Node root_;
        std::deque<Node*> nodes_stack_;
    };

    class BuilderHelper {
    public:
        BuilderHelper(Builder& builder);
        ~BuilderHelper() = default;

        KeyBuilder Key(const std::string& key);
        Builder& Value(const json::Node& value);
        DictBuilder StartDict();
        Builder& EndDict();
        ArrayBuilder StartArray();
        Builder& EndArray();
        Node Build();
    protected:
        Builder& builder_;
    };

    class KeyBuilder : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        DictBuilder Value(const json::Node& value);
        KeyBuilder Key(const std::string&) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
        Node Build() = delete;
    };

    class ArrayBuilder : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        ArrayBuilder Value(const json::Node& value);
        KeyBuilder Key(const std::string&) = delete;
        Builder& EndDict() = delete;
        Node Build() = delete;
    };

    class DictBuilder : public BuilderHelper {
    public:
        using BuilderHelper::BuilderHelper;

        Builder& Value(const json::Node&) = delete;
        DictBuilder StartDict() = delete;
        ArrayBuilder StartArray() = delete;
        Builder& EndArray() = delete;
        Node Build() = delete;
    };

} //namespace json