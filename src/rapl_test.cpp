#include <iostream>

#include <stash/rapl/reader.hpp>
#include <stash/rapl/power.hpp>
#include <stash/util/time.hpp>

#include <algorithm>
#include <unordered_set>
#include <random>
#include <unistd.h>

using namespace stash;

volatile int x;

rapl::power test_add(const rapl::reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x += a[i];
    }
    return rapl::power(r.read() - e0, time() - t0);
}

rapl::power test_xor(const rapl::reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x ^= a[i];
    }
    return rapl::power(r.read() - e0, time() - t0);
}

rapl::power test_mul(const rapl::reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x *= a[i];
    }
    return rapl::power(r.read() - e0, time() - t0);
}

rapl::power test_sleep(const rapl::reader& r, size_t iterations = 5) {
    rapl::power p;
    for(size_t i = 0; i < iterations; i++) {
        auto t0 = time();
        auto e0 = r.read();
        usleep(250'000);
        p += rapl::power(r.read() - e0, time() - t0);
    }
    return p / double(iterations);
}

int main(int argc, char** argv) {
    rapl::reader r(0);

    const size_t n = 10'000'000;
    const size_t iterations = 100;
    
    int* a = new int[n];

    for(size_t it = 0; it < iterations; it++) {
    
        rapl::power p_random;
        {
            auto t0 = time();
            auto e0 = r.read();
            
            std::random_device rnd;
            std::default_random_engine e(rnd());
            std::uniform_int_distribution<int> uniform_dist(1, n);
            for(size_t i = 0; i < n; i++) {
                a[i] = uniform_dist(e);
            }

            p_random = rapl::power(r.read() - e0, time() - t0);
        }

        auto p_sleep = test_sleep(r);
        auto p_add = test_add(r, a, n);
        auto p_xor = test_xor(r, a, n);
        auto p_mul = test_mul(r, a, n);

        std::cout << "RESULT op=sleep it=" << (it+1) << " power=" << p_sleep.total() << std::endl;
        std::cout << "RESULT op=rnd it=" << (it+1) << " power=" << p_random.total() << " power_denoise=" << (p_random.total() - p_sleep.total()) << std::endl;
        std::cout << "RESULT op=add it=" << (it+1) << " power=" << p_add.total() << " power_denoise=" << (p_add.total() - p_sleep.total()) << std::endl;
        std::cout << "RESULT op=xor it=" << (it+1) << " power=" << p_xor.total() << " power_denoise=" << (p_xor.total() - p_sleep.total()) << std::endl;
        std::cout << "RESULT op=mul it=" << (it+1) << " power=" << p_mul.total() << " power_denoise=" << (p_mul.total() - p_sleep.total()) << std::endl;
    }

    delete[] a;
    return 0;
}
