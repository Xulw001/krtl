#ifndef _UNORDERED_MAP_H
#define _UNORDERED_MAP_H

#include "hash.h"
#include "list.h"
#include "vector.h"

namespace rtl {

//////////////////////////////////////////////////////////////////////////
//
// unordered_map
//

template <typename K, typename V>
struct pair {
    K first = {};
    V second = {};

    pair() { ; }
    pair(const K& x, const V& y) : first(x), second(y) { ; }
    pair(const pair& y) : first(y.first), second(y.second) { ; }
    ~pair() { ; }
};

///
/// std::unordered_map analog.
///
/// @tparam K - key type.
/// @tparam V - value type.
/// @tparam Alloc - allocation.
///
template <typename K, typename V, typename Hasher = hash<K>, typename Alloc = allocator<pair<K, V>, PoolTag::Paged>>
class unordered_map {
   private:
    using my_list = typename list<pair<K, V>, Alloc>;
    using iterator = typename my_list::iterator;
    using const_iterator = typename my_list::const_iterator;
    using my_vector = typename vector<iterator, allocator<iterator, pool_tag<Alloc>::value>>;
    using hasher = typename Hasher;

   public:
    using value_type = typename my_list::value_type;
    using size_type = typename my_list::size_type;

    unordered_map() { init(kMinBuckets); }

    template <typename... _Valty>
    pair<iterator, bool> emplace(_Valty&&... val) {
        hash_list_.emplace_front(val...);
        return insert(hash_list_.front(), begin());
    }

    iterator begin() { return hash_list_.begin(); }

    iterator end() { return hash_list_.end(); }

    const_iterator begin() const { return hash_list_.begin(); }

    const_iterator end() const { return hash_list_.end(); }

    iterator find(const K& key) { return find<iterator>(key); }

    const_iterator find(const K& key) const { return find<const_iterator>(key); }

    size_type erase(const K& key) {
        auto it = find(key);
        if (it != end()) {
            erase(it);
            return 1;
        }
        return 0;
    }

    const_iterator erase(const_iterator it) { return erase<const_iterator>(it); }

    iterator erase(iterator it) { return erase<iterator>(it); }

    V& operator[](K& key) { return try_emplace(key).first->second; }

    V& operator[](const K& key) { return try_emplace(key).first->second; }

    void clear() {
        hash_list_.clear();
        init(kMinBuckets);
    }

    size_type bucket_count() const { return buckets_; }

   private:
    template <typename Iterator>
    Iterator find(const K& key) {
        size_type bucket = hash(key);
        for (Iterator it = begin(bucket); it != end(bucket); ++it) {
            if (it->first == key) {
                return it;
            }
        }
        return end();
    }

    template <typename Iterator>
    Iterator erase(Iterator& it) {
        size_type bucket = hash(it->first);
        remove_bucket(it, bucket);
        return hash_list_.erase(it);
    }

    template <typename Key>
    pair<iterator, bool> try_emplace(Key&& key) {
        iterator where = find(key);
        if (where == end()) {
            return emplace(value_type(key, {}));
        }
        return pair<iterator, bool>(where, false);
    }

    void init(size_type buckets) {
        // store begin and end iterator for each bucket
        hash_table_.reserve(buckets * 2);
        hash_table_.assign(buckets * 2, hash_list_.end());
        buckets_ = buckets;
        mask_ = buckets - 1;
    }

    iterator begin(size_type bucket) { return hash_table_[bucket * 2]; }
    const_iterator begin(size_type bucket) const { return (const_iterator&)hash_table_[bucket * 2]; }

    iterator end(size_type bucket) {
        if (hash_table_[bucket * 2] == hash_list_.end()) {
            return hash_list_.end();
        } else {
            iterator end = hash_table_[bucket * 2 + 1];
            return (++end);
        }
    }

    const_iterator end(size_type bucket) const {
        if (hash_table_[bucket * 2] == hash_list_.end()) {
            return hash_list_.end();
        } else {
            const_iterator end =
                (const_iterator&)hash_table_[bucket * 2 + 1];
            return (++end);
        }
    }

    void reinsert() {
        iterator last = end();
        if (last != begin()) {
            for (--last;;) {
                iterator cur = begin();
                bool done = cur == last;
                insert(*cur, cur);
                if (done) break;
            }
        }
    }

    size_type hash(const K& key) const { return hasher()(key) & mask_; }

    void insert_bucket(iterator data, iterator where, size_type bucket) {
        if (hash_table_[bucket * 2] == hash_list_.end()) {
            hash_table_[bucket * 2] = data;
            hash_table_[bucket * 2 + 1] = data;
        } else if (hash_table_[bucket * 2] == where) {
            hash_table_[bucket * 2] = data;
        } else {
            ;  // condition
        }
    }

    void remove_bucket(iterator where, size_type bucket) {
        if (hash_table_[bucket * 2] == where) {
            if (hash_table_[bucket * 2 + 1] == where) {
                hash_table_[bucket * 2] = hash_list_.end();
                hash_table_[bucket * 2 + 1] = hash_list_.end();
            } else {
                hash_table_[bucket * 2] = ++where;
            }
        } else {
            if (hash_table_[bucket * 2 + 1] == where) {
                hash_table_[bucket * 2 + 1] = --where;
            }
        }
    }

    pair<iterator, bool> insert(value_type& val, iterator node) {
        size_type bucket = hash(val.first);
        iterator where = end(bucket);
        while (where != begin(bucket)) {
            if ((--where)->first == val.first) {
                hash_list_.erase(node);
                return pair<iterator, bool>(where, false);
            }
        }

        iterator next = node;
        if (where != ++next) {
            hash_list_.splice(where, hash_list_, node, next);
        }

        insert_bucket(node, where, bucket);
        desired_grow_bucket_count();
        return pair<iterator, bool>(node, true);
    }

    void desired_grow_bucket_count() {
        if (max_bucket_size_ * bucket_count() < hash_list_.size()) {
            size_t buckets = bucket_count();
            if (buckets < 512)
                buckets *= 8;  // multiply by 8
            else if (buckets < SIZE_MAX / 2)
                buckets *= 2;  // multiply safely by 2

            init(buckets);
            reinsert();
        }
    }

   private:
    static constexpr size_type kMinBuckets = 8;  // must be a positive power of 2

    my_list hash_list_;
    my_vector hash_table_;
    size_type mask_;
    size_type buckets_;

    size_type max_bucket_size_ = 1;
};

}  // namespace rtl

/// @}

#endif