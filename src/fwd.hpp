#pragma once

#include <memory>

template <typename T>
class Sequence;

template <typename T>
using SequencePtr = std::shared_ptr<Sequence<T>>;

template <typename T>
class ISortedSequence;

template <typename T>
using SortedSequencePtr = std::shared_ptr<ISortedSequence<T>>;

template <typename Key, typename Value>
class IDictionary;

template <typename Key, typename Value>
using IDictionaryPtr = std::shared_ptr<IDictionary<Key, Value>>;
