#pragma once

#include <functional>
#include <stdexcept>
#include <utility>

#include "array_sequence.hpp"
#include "idictionary.hpp"
#include "list_sequence.hpp"

template <typename Key, typename Value>
class HashTableIterator : public IIterator<KeyValue<Key, Value>> {
    using KeyValuePtr = std::shared_ptr<KeyValue<Key, Value>>;
    using ChainPtr = SequencePtr<KeyValuePtr>;

public:
    explicit HashTableIterator(SequencePtr<ChainPtr> table) : table_(std::move(table)) {
        if (table_ != nullptr) {
            table_it_ = table_->GetIterator();
            AdvanceToNextNonEmptyChain(false);
        }
    }

    bool HasNext() const override {
        return chain_it_ != nullptr && chain_it_->HasNext();
    }

    bool Next() override {
        if (chain_it_ == nullptr) {
            return false;
        }
        chain_it_->Next();
        if (chain_it_->HasNext()) {
            return true;
        }
        return AdvanceToNextNonEmptyChain(true);
    }

    const KeyValue<Key, Value>& GetCurrentItem() const override {
        if (!HasNext()) {
            throw std::out_of_range("No next element");
        }
        return *chain_it_->GetCurrentItem();
    }

    bool TryGetCurrentItem(KeyValue<Key, Value>& element) const override {
        if (!HasNext()) {
            return false;
        }
        element = *chain_it_->GetCurrentItem();
        return true;
    }

private:
    bool AdvanceToNextNonEmptyChain(bool move_forward) {
        if (move_forward && table_it_ != nullptr) {
            table_it_->Next();
        }
        while (table_it_ != nullptr && table_it_->HasNext()) {
            ChainPtr chain = table_it_->GetCurrentItem();
            if (chain != nullptr) {
                chain_it_ = chain->GetIterator();
                if (chain_it_->HasNext()) {
                    return true;
                }
            }
            table_it_->Next();
        }
        chain_it_.reset();
        return false;
    }

    SequencePtr<ChainPtr> table_;
    IIteratorPtr<ChainPtr> table_it_;
    IIteratorPtr<KeyValuePtr> chain_it_;
};

template <typename Key, typename Value, typename Hasher = std::hash<Key>>
class HashTable : public IDictionary<Key, Value> {
    using KeyValuePtr = std::shared_ptr<KeyValue<Key, Value>>;
    using ChainPtr = SequencePtr<KeyValuePtr>;

    static constexpr size_t kDefaultCapacity = 10;
    static constexpr size_t kFactorNominator = 3;
    static constexpr size_t kFactorDenominator = 4;
    static constexpr size_t kScale = 2;
    static constexpr size_t kMaxChainLength = 10;

public:
    HashTable(Hasher hasher = Hasher()) : HashTable(kDefaultCapacity, std::move(hasher)) {
    }

    HashTable(size_t capacity, Hasher hasher = Hasher())
        : table_(std::make_shared<ArraySequence<ChainPtr>>(capacity + 1)), size_(0), hasher_(std::move(hasher)) {
    }

    size_t GetCount() const override {
        return size_;
    }

    size_t GetCapacity() const override {
        return table_->GetLength();
    }

    const Value& Get(const Key& key) const override {
        size_t ind = hasher_(key) % table_->GetLength();
        ChainPtr chain = table_->Get(ind);
        if (chain == nullptr) {
            throw std::out_of_range("No such key");
        }
        for (auto it = chain->GetIterator(); it->HasNext(); it->Next()) {
            auto cur = it->GetCurrentItem();
            if (cur->key == key) {
                return cur->value;
            }
        }
        throw std::out_of_range("No such key");
    }

    bool ContainsKey(const Key& key) const override {
        size_t ind = hasher_(key) % table_->GetLength();
        ChainPtr chain = table_->Get(ind);
        if (chain == nullptr) {
            return false;
        }
        for (auto it = chain->GetIterator(); it->HasNext(); it->Next()) {
            auto cur = it->GetCurrentItem();
            if (cur->key == key) {
                return true;
            }
        }
        return false;
    }

    void Add(const Key& key, const Value& value) override {
        Rehash();
        size_t ind = hasher_(key) % table_->GetLength();
        ChainPtr chain = table_->Get(ind);
        if (chain == nullptr) {
            chain = std::make_shared<ListSequence<KeyValuePtr>>();
            table_->Set(chain, ind);
        }
        for (auto it = chain->GetIterator(); it->HasNext(); it->Next()) {
            auto cur = it->GetCurrentItem();
            if (cur->key == key) {
                cur->value = value;
                return;
            }
        }
        if (chain->GetLength() + 1 >= kMaxChainLength) {
            rehash_requested_ = true;
        }
        chain->Append(std::make_shared<KeyValue<Key, Value>>(key, value));
        ++size_;
    }

    void Remove(const Key& key) override {
        size_t ind = hasher_(key) % table_->GetLength();
        ChainPtr chain = table_->Get(ind);
        if (chain == nullptr) {
            throw std::out_of_range("No such key");
        }
        size_t pos = 0;
        bool found = false;
        {
            size_t i = 0;
            for (auto it = chain->GetIterator(); it->HasNext(); it->Next(), ++i) {
                auto cur = it->GetCurrentItem();
                if (cur->key == key) {
                    pos = i;
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            throw std::out_of_range("No such key");
        }
        chain->EraseAt(pos);
        if (chain->GetLength() == 0) {
            table_->Set(ChainPtr{}, ind);
        }
        --size_;
    }

    SequencePtr<Key> GetKeys() const override {
        auto res = std::make_shared<ListSequence<Key>>();
        for (auto it = table_->GetIterator(); it->HasNext(); it->Next()) {
            auto chain = it->GetCurrentItem();
            if (chain == nullptr) {
                continue;
            }
            for (auto chain_it = chain->GetIterator(); chain_it->HasNext(); chain_it->Next()) {
                auto item = chain_it->GetCurrentItem();
                res->Append(item->key);
            }
        }
        return res;
    }

    SequencePtr<Value> GetValues() const override {
        auto res = std::make_shared<ListSequence<Value>>();
        for (auto it = table_->GetIterator(); it->HasNext(); it->Next()) {
            auto chain = it->GetCurrentItem();
            if (chain == nullptr) {
                continue;
            }
            for (auto chain_it = chain->GetIterator(); chain_it->HasNext(); chain_it->Next()) {
                auto item = chain_it->GetCurrentItem();
                res->Append(item->value);
            }
        }
        return res;
    }

    IIteratorPtr<KeyValue<Key, Value>> GetIterator() const override {
        return std::make_shared<HashTableIterator<Key, Value>>(table_);
    }

private:
    void Rehash() {
        bool need_rehash = rehash_requested_ || (size_ * kFactorDenominator >= table_->GetLength() * kFactorNominator);
        if (!need_rehash) {
            return;
        }
        size_t new_capacity = kScale * table_->GetLength();
        auto new_table = std::make_shared<ArraySequence<ChainPtr>>(new_capacity);
        for (auto it = table_->GetIterator(); it->HasNext(); it->Next()) {
            auto chain = it->GetCurrentItem();
            if (chain == nullptr) {
                continue;
            }
            for (auto chain_it = chain->GetIterator(); chain_it->HasNext(); chain_it->Next()) {
                auto item = chain_it->GetCurrentItem();
                auto ind = hasher_(item->key) % new_capacity;
                ChainPtr dest_chain = new_table->Get(ind);
                if (dest_chain == nullptr) {
                    dest_chain = std::make_shared<ListSequence<KeyValuePtr>>();
                    new_table->Set(dest_chain, ind);
                }
                dest_chain->Append(std::move(item));
            }
        }
        table_ = new_table;
        rehash_requested_ = false;
    }

private:
    SequencePtr<ChainPtr> table_;
    size_t size_;
    bool rehash_requested_ = false;
    const Hasher hasher_;
};
