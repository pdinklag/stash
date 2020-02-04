#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "linear_probing.hpp"

namespace stash {
namespace hash {

template<typename K>
class table {
public:
    using hash_func_t  = std::function<size_t(K)>;
    using probe_func_t = std::function<size_t(size_t)>;

private:
    hash_func_t m_hash_func;
    probe_func_t m_probe_func;
    
    size_t m_cap;
    size_t m_size;
    size_t m_probe_max;
    double m_load_factor;
    double m_growth_factor;

    std::vector<bool> m_used;
    std::vector<K>    m_keys;

    // caches to avoid floating point computations on each insert
    size_t m_size_max;
    size_t m_size_grow;

    // diagnostics
    size_t m_probe_total;
    size_t m_times_resized;

    inline void init(const size_t capacity) {
        m_size = 0;
        m_cap = capacity;
        m_probe_max = 0;
        m_probe_total = 0;
        
        m_used = std::vector<bool>(m_cap);
        m_keys = std::vector<K>(m_cap);

        m_size_max = size_t(m_load_factor * (double)m_cap);
        m_size_grow = std::max(m_size_max + 1, size_t((double)m_cap * m_growth_factor));
    }

    inline size_t hash(const K& key) const {
        return size_t(m_hash_func(key)) % m_cap;
    }

    inline void insert_internal(const K& key) {
        const size_t hkey = hash(key);
        
        size_t h = hkey;
        size_t i = 0;
        size_t probe = 0;
        
        while(m_used[h]) {
            i = m_probe_func(i);
            h = (hkey + i) % m_cap;
            ++probe;
        }

        m_probe_total += probe;
        m_probe_max = std::max(m_probe_max, probe);
        
        m_used[h] = 1;
        m_keys[h] = key;
        ++m_size;
    }

    inline void resize(const size_t new_cap) {
        ++m_times_resized;
        
        auto old_cap = m_cap;
        auto used = m_used;
        auto keys = m_keys;

        init(new_cap);

        for(size_t i = 0; i < old_cap; i++) {
            if(used[i]) insert_internal(keys[i]);
        }
    }

public:
    inline table(
        hash_func_t hash_func,
        size_t capacity,
        double load_factor = 1.0,
        double growth_factor = 2.0,
        probe_func_t probe_func = linear_probing<>{})
        : m_hash_func(hash_func),
          m_load_factor(load_factor),
          m_growth_factor(growth_factor),
          m_probe_func(probe_func),
          m_times_resized(0) {

        init(capacity);
    }

    inline size_t size() const {
        return m_size;
    }
    
    inline size_t capacity() const {
        return m_cap;
    }

    inline double load() const {
        return (double)m_size / (double)m_cap;
    }

    inline size_t max_probe() const {
        return m_probe_max;
    }

    inline double avg_probe() const {
        return (double)m_probe_total / (double)m_size;
    }

    inline size_t times_resized() const {
        return m_times_resized;
    }

    inline void insert(const K& key) {
        // first, check if growing is necessary
        if(m_size + 1 > m_size_max) {
            resize(m_size_grow);
        }
        
        // now it's safe to insert
        insert_internal(key);
    }

    inline bool contains(const K& key) const {
        const size_t hkey = hash(key);
        
        size_t h = hkey;
        if(m_used[h] && m_keys[h] == key) {
            return true;
        } else {
            size_t i = 0;
            for(size_t probe = 0; probe < m_probe_max; probe++) {
                i = m_probe_func(i);
                h = (hkey + i) % m_cap;
                if(m_used[h]) {
                    if(m_keys[h] == key) return true;
                } else {
                    return false; // key cannot be contained
                }
            }
            return false; // key not found
        }
    }
};

}}
