#include <algorithm>
#include <functional>
#include <vector>

namespace stash {
namespace hash {

template<typename K>
class table {
public:
    using hash_func_t = std::function<size_t(K)>;

private:
    hash_func_t m_hash_func;
    size_t m_cap;
    size_t m_size;
    size_t m_probe_max;
    double m_load_factor;
    double m_growth_factor;
    
    std::vector<bool> m_used;
    std::vector<K>    m_keys;

    inline size_t hash(const K& key) const {
        return size_t(m_hash_func(key)) % m_cap;
    }

    inline void insert_internal(const K& key) {
        size_t probe = 0;
        size_t h = hash(key);
        while(m_used[h]) {
            h = (h + 1) % m_cap;
            ++probe;
        }
        
        m_probe_max = std::max(m_probe_max, probe);
        m_used[h] = 1;
        m_keys[h] = key;
        ++m_size;
    }

    inline void rehash(size_t new_cap) {
        auto old_cap = m_cap;
        auto used = m_used;
        auto keys = m_keys;

        m_size = 0;
        m_cap = new_cap;
        m_probe_max = 0;
        
        m_used = std::vector<bool>(m_cap);
        m_keys = std::vector<K>(m_cap);

        for(size_t i = 0; i < old_cap; i++) {
            if(used[i]) insert_internal(keys[i]);
        }
    }

public:
    inline table(hash_func_t hash_func, size_t capacity, double load_factor = 1.0, double growth_factor = 2.0)
        : m_hash_func(hash_func),
          m_cap(capacity),
          m_size(0),
          m_probe_max(0),
          m_load_factor(load_factor),
          m_growth_factor(growth_factor) {

        m_used = std::vector<bool>(m_cap);
        m_keys = std::vector<K>(m_cap);
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

    inline void insert(const K& key) {
        // first, check if a resize is necessary
        const double load = (double)(m_size + 1) / (double)m_cap;
        if(load > m_load_factor) {
            size_t new_cap = size_t((double)m_cap * m_growth_factor);
            rehash(new_cap > m_cap ? new_cap : m_cap + 1);
        }
        
        // now it's safe to insert
        insert_internal(key);
    }

    inline bool contains(const K& key) const {
        size_t h = hash(key);
        if(m_used[h] && m_keys[h] == key) {
            return true;
        } else {
            for(size_t probe = 0; probe < m_probe_max; probe++) {
                h = (h + 1) % m_cap;
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
