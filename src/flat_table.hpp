#pragma once

#include <stdexcept>

#include "idictionary.hpp"
#include "isorted_sequence.hpp"
#include "list_sequence.hpp"
#include "sorted_sequence.hpp"

template <typename Key, typename Value>
class FlatTable : public IDictionary<Key, Value> {
    struct KeyCompare {
        bool operator()(const KeyValue<Key, Value>& a, const KeyValue<Key, Value>& b) const {
            return a.key < b.key;
        }
    };

    using Pair = KeyValue<Key, Value>;
    using Seq = SortedSequence<Pair, KeyCompare>;

public:
    FlatTable() : data_(std::make_shared<Seq>()) {
    }

    size_t GetCount() const override {
        return data_->GetLength();
    }

    size_t GetCapacity() const override {
        return data_->GetLength();
    }

    const Value& Get(const Key& key) const override {
        size_t idx = LowerIndex(key);
        if (idx == data_->GetLength() || data_->Get(idx).key != key) {
            throw std::out_of_range("No such key");
        }
        return data_->Get(idx).value;
    }

    bool ContainsKey(const Key& key) const override {
        size_t idx = LowerIndex(key);
        return idx < data_->GetLength() && data_->Get(idx).key == key;
    }

    void Add(const Key& key, const Value& value) override {
        size_t idx = LowerIndex(key);
        if (idx < data_->GetLength() && data_->Get(idx).key == key) {
            Pair updated{key, value};
            data_->EraseAt(idx);
            data_->Add(updated);
            return;
        }
        data_->Add(Pair{key, value});
    }

    void Remove(const Key& key) override {
        size_t idx = LowerIndex(key);
        if (idx == data_->GetLength() || data_->Get(idx).key != key) {
            throw std::out_of_range("No such key");
        }
        data_->EraseAt(idx);
    }

    SequencePtr<Key> GetKeys() const override {
        auto res = std::make_shared<ListSequence<Key>>();
        for (auto it = data_->GetIterator(); it->HasNext(); it->Next()) {
            res->Append(it->GetCurrentItem().key);
        }
        return res;
    }

    SequencePtr<Value> GetValues() const override {
        auto res = std::make_shared<ListSequence<Value>>();
        for (auto it = data_->GetIterator(); it->HasNext(); it->Next()) {
            res->Append(it->GetCurrentItem().value);
        }
        return res;
    }

    IIteratorPtr<Pair> GetIterator() const override {
        return data_->GetIterator();
    }

private:
    size_t LowerIndex(const Key& key) const {
        return data_->LowerBound(Pair{key, Value{}});
    }

private:
    std::shared_ptr<Seq> data_;
};
