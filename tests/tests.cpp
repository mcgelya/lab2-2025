#include <catch2/catch_test_macros.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "alphabet_index.hpp"
#include "array_sequence.hpp"
#include "flat_table.hpp"
#include "hash_table.hpp"
#include "list_sequence.hpp"
#include "sorted_sequence.hpp"

template <typename T>
std::vector<T> ToVector(const Sequence<T>& seq) {
    std::vector<T> res;
    for (auto it = seq.GetIterator(); it->HasNext(); it->Next()) {
        res.push_back(it->GetCurrentItem());
    }
    return res;
}

template <typename T>
std::vector<T> ToVector(const SequencePtr<T>& seq) {
    return ToVector(*seq);
}

template <typename T>
std::vector<T> ToVector(const ISortedSequence<T>& seq) {
    std::vector<T> res;
    for (auto it = seq.GetIterator(); it->HasNext(); it->Next()) {
        res.push_back(it->GetCurrentItem());
    }
    return res;
}

template <typename T>
std::vector<T> ToVector(const SortedSequencePtr<T>& seq) {
    return ToVector(*seq);
}

template <typename K, typename V>
std::vector<KeyValue<K, V>> ToPairs(const IDictionary<K, V>& dict) {
    std::vector<KeyValue<K, V>> res;
    auto it = dict.GetIterator();
    while (it->HasNext()) {
        res.push_back(it->GetCurrentItem());
        it->Next();
    }
    return res;
}

TEST_CASE("ArraySeq") {
    int init[]{1, 2, 3};
    ArraySequence<int> seq(init, 3);

    SECTION("Ops") {
        seq.Append(4);
        seq.Prepend(0);
        seq.InsertAt(99, 2);

        REQUIRE(seq.GetLength() == 6);
        REQUIRE(seq.Get(0) == 0);
        REQUIRE(seq.Get(2) == 99);

        seq.EraseAt(2);
        REQUIRE(seq.GetLength() == 5);
        REQUIRE(ToVector(seq) == std::vector<int>({0, 1, 2, 3, 4}));
    }

    SECTION("Slices") {
        auto sub = seq.GetSubsequence(1, 2);
        REQUIRE(ToVector(sub) == std::vector<int>({2, 3}));

        auto first = seq.GetFirst(2);
        REQUIRE(ToVector(first) == std::vector<int>({1, 2}));

        auto last = seq.GetLast(2);
        REQUIRE(ToVector(last) == std::vector<int>({2, 3}));
    }

    SECTION("Clear") {
        seq.Clear();
        REQUIRE(seq.GetLength() == 0);
        seq.Append(7);
        REQUIRE(ToVector(seq) == std::vector<int>({7}));
    }

    SECTION("Bounds") {
        REQUIRE_THROWS_AS(seq.EraseAt(5), std::out_of_range);
    }

    SECTION("Cap") {
        auto start_cap = seq.GetCapacity();
        for (int i = 0; i < 20; ++i) {
            seq.Append(100 + i);
        }
        REQUIRE(seq.GetLength() == 23);
        REQUIRE(seq.GetCapacity() >= seq.GetLength());
        REQUIRE(seq.GetCapacity() > start_cap);
        REQUIRE(seq.Get(22) == 119);
    }
}

TEST_CASE("ListSeq") {
    int init[]{10, 20, 30};
    ListSequence<int> seq(init, 3);

    seq.Append(40);
    seq.Prepend(0);
    seq.InsertAt(15, 2);
    REQUIRE(ToVector(seq) == std::vector<int>({0, 10, 15, 20, 30, 40}));
    REQUIRE(seq.GetFirst() == 0);
    REQUIRE(seq.GetLast() == 40);

    seq.EraseAt(2);
    REQUIRE(ToVector(seq) == std::vector<int>({0, 10, 20, 30, 40}));
    REQUIRE(seq.GetLength() == 5);

    auto last = seq.GetLast(2);
    REQUIRE(ToVector(last) == std::vector<int>({30, 40}));

    auto first = seq.GetFirst(3);
    REQUIRE(ToVector(first) == std::vector<int>({0, 10, 20}));

    seq.Clear();
    REQUIRE(seq.GetLength() == 0);
    REQUIRE_THROWS_AS(seq.GetFirst(), std::out_of_range);

    SECTION("Erase") {
        ListSequence<int> local(init, 3);
        local.EraseAt(0);
        REQUIRE(ToVector(local) == std::vector<int>({20, 30}));
        local.EraseAt(local.GetLength() - 1);
        REQUIRE(ToVector(local) == std::vector<int>({20}));
    }
}

TEST_CASE("HashPut") {
    HashTable<int, int> table;
    for (int i = 0; i < 9; ++i) {
        table.Add(i, i * i);
    }
    REQUIRE(table.GetCount() == 9);

    SECTION("Get") {
        REQUIRE(table.ContainsKey(4));
        REQUIRE(table.Get(4) == 16);
        REQUIRE_FALSE(table.ContainsKey(99));
        REQUIRE_THROWS_AS(table.Get(99), std::out_of_range);
    }

    SECTION("Upd") {
        table.Add(4, 999);
        REQUIRE(table.Get(4) == 999);
        REQUIRE(table.GetCount() == 9);
    }

    SECTION("Rm") {
        table.Remove(3);
        REQUIRE_FALSE(table.ContainsKey(3));
        REQUIRE(table.GetCount() == 8);
        REQUIRE_THROWS_AS(table.Remove(3), std::out_of_range);
    }
}

TEST_CASE("HashIter") {
    HashTable<int, int> table;
    for (int i = 1; i <= 12; ++i) {
        table.Add(i, i + 100);
    }

    std::unordered_map<int, int> seen;
    auto it = table.GetIterator();
    while (it->HasNext()) {
        auto kv = it->GetCurrentItem();
        seen[kv.key] = kv.value;
        it->Next();
    }

    REQUIRE(seen.size() == table.GetCount());
    for (int i = 1; i <= 12; ++i) {
        REQUIRE(seen[i] == i + 100);
    }

    auto keys = ToVector(table.GetKeys());
    auto values = ToVector(table.GetValues());
    REQUIRE(keys.size() == table.GetCount());
    REQUIRE(values.size() == table.GetCount());

    std::unordered_set<int> key_set(keys.begin(), keys.end());
    REQUIRE(key_set.size() == table.GetCount());
    for (int i = 1; i <= 12; ++i) {
        REQUIRE(key_set.contains(i));
    }
}

TEST_CASE("HashRehash") {
    HashTable<int, int> table(5);
    auto initial_cap = table.GetCapacity();

    const int total = 40;
    for (int i = 0; i < total; ++i) {
        table.Add(i, i * 2);
    }

    REQUIRE(table.GetCount() == total);
    REQUIRE(table.GetCapacity() > initial_cap);  // must have rehashed

    for (int i = 0; i < total; ++i) {
        REQUIRE(table.ContainsKey(i));
        REQUIRE(table.Get(i) == i * 2);
    }
}

TEST_CASE("HashCol") {
    struct BadHasher {
        size_t operator()(int) const {
            return 1;
        }
    };

    HashTable<int, int, BadHasher> table(3, BadHasher{});
    table.Add(1, 10);
    table.Add(2, 20);
    table.Add(3, 30);
    table.Add(2, 200);

    REQUIRE(table.Get(2) == 200);
    REQUIRE(table.GetCount() == 3);

    table.Remove(1);
    REQUIRE_FALSE(table.ContainsKey(1));
    REQUIRE(table.GetCount() == 2);

    // Remaining elements still retrievable
    REQUIRE(table.Get(2) == 200);
    REQUIRE(table.Get(3) == 30);

    // Ensure iterator still walks surviving items
    std::unordered_set<int> keys_seen;
    auto it = table.GetIterator();
    while (it->HasNext()) {
        keys_seen.insert(it->GetCurrentItem().key);
        it->Next();
    }
    REQUIRE(keys_seen == std::unordered_set<int>({2, 3}));
}

TEST_CASE("SortedSeq") {
    int init[]{5, 1, 3, 2, 4};
    SortedSequence<int> seq(init, 5);

    SECTION("Order") {
        REQUIRE(ToVector(seq) == std::vector<int>({1, 2, 3, 4, 5}));
        REQUIRE(seq.GetFirst() == 1);
        REQUIRE(seq.GetLast() == 5);
        REQUIRE(seq.GetLength() == 5);
        REQUIRE_FALSE(seq.GetIsEmpty());
    }

    SECTION("AddRemove") {
        seq.Add(0);
        seq.Add(6);
        REQUIRE(ToVector(seq) == std::vector<int>({0, 1, 2, 3, 4, 5, 6}));
        REQUIRE(seq.Remove(3));
        REQUIRE_FALSE(seq.Remove(42));
        REQUIRE(ToVector(seq) == std::vector<int>({0, 1, 2, 4, 5, 6}));
        seq.EraseAt(0);
        REQUIRE(seq.GetFirst() == 1);
        seq.Clear();
        REQUIRE(seq.GetIsEmpty());
    }

    SECTION("Bounds") {
        REQUIRE(seq.LowerBound(0) == 0);
        REQUIRE(seq.LowerBound(3) == 2);
        REQUIRE(seq.LowerBound(6) == seq.GetLength());
        REQUIRE(seq.IndexOf(4) == 3);
        REQUIRE(seq.IndexOf(9) == -1);
    }

    SECTION("Subseq") {
        auto sub = seq.GetSubsequence(1, 3);
        REQUIRE(ToVector(sub) == std::vector<int>({2, 3, 4}));
    }
}

TEST_CASE("FlatTable") {
    FlatTable<int, int> dict;

    SECTION("Put") {
        dict.Add(2, 20);
        dict.Add(1, 10);
        dict.Add(3, 30);

        REQUIRE(dict.GetCount() == 3);
        REQUIRE(dict.Get(1) == 10);
        REQUIRE(dict.Get(2) == 20);
        REQUIRE(dict.Get(3) == 30);
    }

    SECTION("Upd") {
        dict.Add(1, 10);
        dict.Add(1, 100);
        REQUIRE(dict.Get(1) == 100);
        REQUIRE(dict.GetCount() == 1);
    }

    SECTION("Rm") {
        dict.Add(1, 10);
        dict.Add(2, 20);
        dict.Remove(1);
        REQUIRE_FALSE(dict.ContainsKey(1));
        REQUIRE(dict.GetCount() == 1);
        REQUIRE_THROWS_AS(dict.Remove(1), std::out_of_range);
    }

    SECTION("Iter") {
        dict.Add(2, 20);
        dict.Add(1, 10);
        dict.Add(3, 30);
        auto keys = ToVector(dict.GetKeys());
        auto vals = ToVector(dict.GetValues());
        REQUIRE(keys == std::vector<int>({1, 2, 3}));
        REQUIRE(vals == std::vector<int>({10, 20, 30}));

        auto pairs = ToPairs(dict);
        REQUIRE(pairs.front().key == 1);
        REQUIRE(pairs.back().key == 3);
    }

    SECTION("GetFail") {
        dict.Add(2, 20);
        REQUIRE(dict.ContainsKey(2));
        REQUIRE_FALSE(dict.ContainsKey(99));
        REQUIRE_THROWS_AS(dict.Get(99), std::out_of_range);
    }

    SECTION("Order") {
        dict.Add(3, 30);
        dict.Add(1, 10);
        dict.Add(2, 20);
        dict.Add(2, 200);  // update existing
        auto pairs = ToPairs(dict);
        REQUIRE(pairs.size() == 3);
        REQUIRE(pairs[0].key == 1);
        REQUIRE(pairs[1].key == 2);
        REQUIRE(pairs[1].value == 200);
        REQUIRE(pairs[2].key == 3);
    }
}

TEST_CASE("AIndexWords") {
    std::string text = "alpha beta gamma delta epsilon";
    // page_size in words = 4, first page half -> 2 words
    auto dict = BuildAlphabetIndex<HashTable<std::string, int>>(text, 4, AlphabetIndexMode::Words);
    REQUIRE(dict->Get("alpha") == 1);
    REQUIRE(dict->Get("beta") == 1);
    REQUIRE(dict->Get("gamma") == 2);
    REQUIRE(dict->Get("delta") == 2);
    REQUIRE(dict->Get("epsilon") == 2);
}

TEST_CASE("AIndexChars") {
    std::string text = "aa bbb c ddd";
    // page_size chars = 6, first page half -> 3 chars
    auto dict = BuildAlphabetIndex<FlatTable<std::string, int>>(text, 6, AlphabetIndexMode::Chars);
    REQUIRE(dict->Get("aa") == 1);
    REQUIRE(dict->Get("bbb") == 2);
    REQUIRE(dict->Get("c") == 2);
    REQUIRE(dict->Get("ddd") == 3);
}

TEST_CASE("AIndexEmpty") {
    std::string text;
    auto dict = BuildAlphabetIndex<HashTable<std::string, int>>(text, 10, AlphabetIndexMode::Words);
    REQUIRE(dict->GetCount() == 0);
    REQUIRE(ToVector(dict->GetKeys()).empty());
}

TEST_CASE("AIndexRepeat") {
    std::string text = "a b c a d";
    auto dict = BuildAlphabetIndex<FlatTable<std::string, int>>(text, 2, AlphabetIndexMode::Words);
    REQUIRE(dict->Get("a") == 1);  // repeated later, keep first page
    REQUIRE(dict->Get("b") == 2);
    REQUIRE(dict->Get("c") == 2);
}

TEST_CASE("AIndexTiny") {
    std::string text = "aa bb c";
    auto dict = BuildAlphabetIndex<HashTable<std::string, int>>(text, 1, AlphabetIndexMode::Chars);
    REQUIRE(dict->Get("aa") == 1);
    REQUIRE(dict->Get("bb") == 2);
    REQUIRE(dict->Get("c") == 3);
}
