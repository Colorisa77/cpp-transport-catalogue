#include "json_builder.h"

namespace json {
    Builder::~Builder() {
        for(const auto& ptr : nodes_stack_) {
            delete ptr;
        }
        nodes_stack_.clear();
    }

    KeyBuilder Builder::Key(const std::string& key) {
        if(nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Dictionary is not started"s);
        }
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsString()) {
            throw std::logic_error("Not allowed use method Key after Key"s);
        }
        Node* new_node = new Node(key);
        nodes_stack_.emplace_back(new_node);
        return KeyBuilder(*this);
    }

    Builder& Builder::Value(const Node& value) {
        if(nodes_stack_.empty()) {
            if(!root_.IsNull()) {
                throw std::logic_error("Value is already set"s);
            }
            root_ = std::move(value);
        } else {
            if(nodes_stack_.back()->IsArray()) {
               const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(value));
            } else if (nodes_stack_.back()->IsDict()) {
                throw std::logic_error("Expected key for dictionary first"s); 
            } else if (nodes_stack_.back()->IsString()) {
                std::string curr_key = std::move(nodes_stack_.back()->AsString());
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[curr_key] = std::move(value);
            } else {
                throw std::logic_error("current node is not Array or Dictionary"s);
            }
        }
        return *this;
    }

    DictBuilder Builder::StartDict() {
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Expected key for dictionary first"s);
        }
        Node* new_node = new Node(Dict{});
        nodes_stack_.emplace_back(new_node);
        return DictBuilder(*this);
    }

    Builder& Builder::EndDict() {
        if(nodes_stack_.empty()) {
            throw std::logic_error("Dict is not started"s);
        }
        if(!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Expected EndArray for array"s);
        }
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsString()) {
            throw std::logic_error("Expected Value or start for container"s);
        }
        if(nodes_stack_.size() == 1 && nodes_stack_.back()->IsDict()) {
            root_ = std::move(*nodes_stack_.back());
            delete nodes_stack_.back();
            nodes_stack_.pop_back();
        } else {
            Dict curr_dict = std::move(nodes_stack_.back()->AsDict());
            delete nodes_stack_.back();
            nodes_stack_.pop_back();
            if(nodes_stack_.back()->IsString()) {
                std::string curr_key = std::move(nodes_stack_.back()->AsString());
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[curr_key] = std::move(curr_dict);
            } else {
                const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(curr_dict));
            }
        }
        return *this;
    }

    ArrayBuilder Builder::StartArray() {
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Expected key for dictionary first"s);
        }
        Node* new_node = new Node(Array{});
        nodes_stack_.emplace_back(new_node);
        return ArrayBuilder(*this);
    }

    Builder& Builder::EndArray() {
        if(nodes_stack_.empty()) {
            throw std::logic_error("Array is not started"s);
        }
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsString()) {
            throw std::logic_error("Expected Value or start for container"s);
        }
        /*if(!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Expected EndArray for array"s);
        }*/
        if(nodes_stack_.size() == 1) {
            root_ = std::move(*nodes_stack_.back());
            delete nodes_stack_.back();
            nodes_stack_.pop_back();
        } else {
            Array curr_arr = std::move(nodes_stack_.back()->AsArray());
            delete nodes_stack_.back();
            nodes_stack_.pop_back();
            if(nodes_stack_.back()->IsString()) {
                std::string curr_key = std::move(nodes_stack_.back()->AsString());
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                const_cast<Dict&>(nodes_stack_.back()->AsDict())[curr_key] = std::move(curr_arr);
            } else {
                const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(std::move(curr_arr));
            }
        }
        return *this;
    }

    Node Builder::Build() {
        if (nodes_stack_.size() > 0) {
            throw std::logic_error("Not all nodes are complite. Check curr_node_ and node_stack_"s);
        }
        if (root_.IsNull() == true) {
            throw std::logic_error("Bulder finished after start"s);
        }
        return root_;
    }

    //=================== BuildHelper ===================//

    BuilderHelper::BuilderHelper(Builder& builder) 
        : builder_(builder) {
    }

    KeyBuilder BuilderHelper::Key(const std::string& key) {
        builder_.Key(key);
        return KeyBuilder(builder_);
    }

    Builder& BuilderHelper::Value(const json::Node& value) {
        builder_.Value(value);
        return builder_;
    }
    
    DictBuilder BuilderHelper::StartDict() {
        builder_.StartDict();
        return DictBuilder(builder_);
    }

    Builder& BuilderHelper::EndDict() {
        builder_.EndDict();
        return builder_;
    }

    ArrayBuilder BuilderHelper::StartArray() {
        builder_.StartArray();
        return ArrayBuilder(builder_);
    }

    Builder& BuilderHelper::EndArray() {
        builder_.EndArray();
        return builder_;
    }

    Node BuilderHelper::Build() {
        return builder_.Build();
    }

    //=================== KeyBuilder ===================//

    DictBuilder KeyBuilder::Value(const json::Node& value) {
        builder_.Value(value);
        return DictBuilder(builder_);
    }

    //=================== ArrayBuilder ===================//

    ArrayBuilder ArrayBuilder::Value(const json::Node& value) {
        builder_.Value(value);
        return ArrayBuilder(builder_);
    }

} //namespace json