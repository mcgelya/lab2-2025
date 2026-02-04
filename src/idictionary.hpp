#pragma once

#include "fwd.hpp"
#include "iiterator.hpp"

template <typename Key, typename Value>
struct KeyValue {
    Key key;
    Value value;
};

template <typename Key, typename Value>
class IDictionary : public IIterable<KeyValue<Key, Value>> {
public:
    virtual ~IDictionary() = default;

    virtual size_t GetCount() const = 0;
    virtual size_t GetCapacity() const = 0;

    virtual const Value& Get(const Key& key) const = 0;
    virtual bool ContainsKey(const Key& key) const = 0;
    virtual void Add(const Key& key, const Value& value) = 0;
    virtual void Remove(const Key& key) = 0;

    virtual SequencePtr<Key> GetKeys() const = 0;
    virtual SequencePtr<Value> GetValues() const = 0;
};
