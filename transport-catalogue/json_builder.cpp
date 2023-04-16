#include "json_builder.h"

namespace json {
    Builder::~Builder() {
        for(const auto& ptr : nodes_stack_) {
            delete ptr;
        }
        nodes_stack_.clear();
    }

    KeyBuilderHelper Builder::Key(const std::string& key) {
        if(stack_of_dict_info_.empty()) {
            throw std::logic_error("Dictionary is not started"s);
        }
        if(!nodes_stack_.empty() && (nodes_stack_.back()->IsDict() && stack_of_dict_info_.back().is_after_key_ == true)) {
            throw std::logic_error("Not allowed use method Key after Key"s);
        }
        curr_key_ = key;
        stack_of_dict_info_.back().is_after_start_dict_ = false;
        stack_of_dict_info_.back().is_after_key_ = true;
        return KeyBuilderHelper(*this);
    }

    Builder& Builder::Value(const Node& value) {
        if(curr_node_ < 0) {
            if(!root_.IsNull()) {
                throw std::logic_error("Value is already set"s);
            }
            root_ = std::move(value);
        } else {
            if(curr_node_ < 0) {
                throw std::logic_error("Array is not started"s);
            }
            if(nodes_stack_[curr_node_]->IsArray() == true) {
               const_cast<Array&>(nodes_stack_[curr_node_]->AsArray()).emplace_back(std::move(value));
            } else if (nodes_stack_[curr_node_]->IsDict() == true) {
                if(stack_of_dict_info_.back().is_after_key_ == false) {
                    throw std::logic_error("Expected key for dictionary first"s);
                }
                const_cast<Dict&>(nodes_stack_[curr_node_]->AsDict())[curr_key_] = std::move(value);
                stack_of_dict_info_.back().is_after_key_ = false;
            } else {
                throw std::logic_error("current node is not Array or Dictionary"s);
            }
        }
        if(!stack_of_dict_info_.empty()) {
            stack_of_dict_info_.back().is_after_key_ = false;
        }
        return *this;
    }

    DictBuilderHelper Builder::StartDict() {
        if(!stack_of_dict_info_.empty() && (stack_of_dict_info_.back().is_after_start_dict_ == true)) {
            throw std::logic_error("Expected key for dictionary first"s);
        }
        if(!nodes_stack_.empty() && (nodes_stack_.back()->IsDict() && stack_of_dict_info_.back().is_after_key_ == false)) {
            throw std::logic_error("Expected key for dictionary first"s);
        }
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            stack_of_dict_info_.back().is_after_key_ = false;
        }
        Node* new_node = new Node(Dict{});
        nodes_stack_.push_back(new_node);
        stack_of_dict_keys_.push_back(curr_key_);
        curr_key_.clear();
        ++curr_node_;
        AfterDict afterdict{};
        stack_of_dict_info_.push_back(afterdict);
        stack_of_dict_info_.back().is_after_start_dict_ = true;
        return DictBuilderHelper(*this);
    }

    Builder& Builder::EndDict() {
        if(nodes_stack_.empty()) {
            throw std::logic_error("Dict is not started"s);
        }
        if(!nodes_stack_.empty() && !nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Expected EndArray for array"s);
        }
        if(stack_of_dict_info_.back().is_after_key_ == true) {
            throw std::logic_error("Expected Value or start for container"s);
        }
        if(curr_node_ == 0) {
            root_ = std::move(*nodes_stack_[curr_node_]);
            delete nodes_stack_[curr_node_];
            nodes_stack_.pop_back();
            --curr_node_;
            curr_key_ = stack_of_dict_keys_.back();
        } else {
            if(nodes_stack_[curr_node_ - 1]->IsDict() == true) {
                Dict& prev_dict = const_cast<Dict&>(nodes_stack_[curr_node_ - 1]->AsDict());
                Dict& curr_dict = const_cast<Dict&>(nodes_stack_[curr_node_]->AsDict());
                prev_dict[stack_of_dict_keys_.back()] = curr_dict;
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                if(stack_of_dict_info_.back().is_after_start_dict_ == false) {
                    stack_of_dict_keys_.pop_back();
                }
                --curr_node_;
                curr_key_ = stack_of_dict_keys_.back();
                stack_of_dict_info_.pop_back();
            } else {
                Array& prev_arr = const_cast<Array&>(nodes_stack_[curr_node_ - 1]->AsArray());
                Dict& curr_dict = const_cast<Dict&>(nodes_stack_[curr_node_]->AsDict());
                prev_arr.emplace_back(curr_dict);
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                --curr_node_;
                curr_key_ = stack_of_dict_keys_.back();
                stack_of_dict_info_.pop_back();
            }
        }
        return *this;
    }

    ArrayBuilderHelper Builder::StartArray() {
        if(!stack_of_dict_info_.empty() && stack_of_dict_info_.back().is_after_start_dict_ == true) {
            throw std::logic_error("Expected key for dictionary first"s);
        }
        if(!nodes_stack_.empty() && (nodes_stack_.back()->IsDict() && stack_of_dict_info_.back().is_after_key_ == false)) {
            throw std::logic_error("Expected key for array first"s);
        }
        if(!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            stack_of_dict_info_.back().is_after_key_ = false;
        }
        Node* new_node = new Node(Array{});
        nodes_stack_.emplace_back(new_node);
        ++curr_node_;
        return ArrayBuilderHelper(*this);
    }

    Builder& Builder::EndArray() {
        if(nodes_stack_.empty()) {
            throw std::logic_error("Array is not started"s);
        }
        if(!stack_of_dict_info_.empty() && (stack_of_dict_info_.back().is_after_key_ == true && !nodes_stack_.back()->IsArray())) {
            throw std::logic_error("Expected Value or start for container"s);
        }
        if(!nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Expected EndArray for array"s);
        }
        if(curr_node_ == 0) {
            root_ = std::move(*nodes_stack_[curr_node_]);
            delete nodes_stack_[curr_node_];
            --curr_node_;
            nodes_stack_.pop_back();
        } else {
            if(nodes_stack_[curr_node_ - 1]->IsDict() == true) {
                Dict& prev_dict = const_cast<Dict&>(nodes_stack_[curr_node_ - 1]->AsDict());
                Array& curr_arr = const_cast<Array&>(nodes_stack_[curr_node_]->AsArray());
                prev_dict[curr_key_] = curr_arr;
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                --curr_node_;
            } else {
                Array& prev_arr = const_cast<Array&>(nodes_stack_[curr_node_ - 1]->AsArray());
                Array& curr_arr = const_cast<Array&>(nodes_stack_[curr_node_]->AsArray());
                prev_arr.emplace_back(curr_arr);
                delete nodes_stack_.back();
                nodes_stack_.pop_back();
                --curr_node_;
            }
        }
        return *this;
    }

    Node Builder::Build() {
        if (nodes_stack_.size() > 0 || curr_node_ != -1) {
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

    KeyBuilderHelper BuilderHelper::Key(const std::string& key) {
        builder_.Key(key);
        return KeyBuilderHelper(builder_);
    }

    Builder& BuilderHelper::Value(const json::Node& value) {
        builder_.Value(value);
        return builder_;
    }
    
    DictBuilderHelper BuilderHelper::StartDict() {
        builder_.StartDict();
        return DictBuilderHelper(builder_);
    }

    Builder& BuilderHelper::EndDict() {
        builder_.EndDict();
        return builder_;
    }

    ArrayBuilderHelper BuilderHelper::StartArray() {
        builder_.StartArray();
        return ArrayBuilderHelper(builder_);
    }

    Builder& BuilderHelper::EndArray() {
        builder_.EndArray();
        return builder_;
    }

    Node BuilderHelper::Build() {
        return builder_.Build();
    }

    //=================== ValueAfterStartArrayBuilderHelper ===================//

    ValueAfterStartArrayBuilderHelper ValueAfterStartArrayBuilderHelper::Value(const json::Node& value) {
        builder_.Value(value);
        return ValueAfterStartArrayBuilderHelper(builder_);
    }

    //=================== KeyBuilderHelper ===================//

    ValueAfterKeyBuilderHelper KeyBuilderHelper::Value(const json::Node& value) {
        builder_.Value(value);
        return ValueAfterKeyBuilderHelper(builder_);
    }

    //=================== ArrayBuilderHelper ===================//

    ValueAfterStartArrayBuilderHelper ArrayBuilderHelper::Value(const json::Node& value) {
        builder_.Value(value);
        return ValueAfterStartArrayBuilderHelper(builder_);
    }

} //namespace json