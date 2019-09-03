#include <iostream>

#include <rapl/reader.hpp>
#include <rapl/power.hpp>
#include <util/time.hpp>

#include <algorithm>
#include <unordered_set>
#include <random>
#include <unistd.h>

rapl::Power noise(const rapl::Reader& r, const size_t num = 5) {
    rapl::Power p;
    for(size_t i = 0; i < num; i++) {
        auto t0 = time();
        auto e0 = r.read();
        
        sleep(1);
        
        p = p + rapl::Power(r.read() - e0, time() - t0);
    }
    
    return rapl::Power(
        p.core   / double(num),
        p.uncore / double(num),
        p.dram   / double(num));
}

volatile int x;

rapl::Power test_add(const rapl::Reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x += a[i];
    }
    return rapl::Power(r.read() - e0, time() - t0);
}

rapl::Power test_xor(const rapl::Reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x ^= a[i];
    }
    return rapl::Power(r.read() - e0, time() - t0);
}

rapl::Power test_mul(const rapl::Reader& r, const int* a, size_t n) {
    auto t0 = time();
    auto e0 = r.read();
    for(size_t i = 0; i < n; i++) {
        x *= a[i];
    }
    return rapl::Power(r.read() - e0, time() - t0);
}

int main(int argc, char** argv) {
    rapl::Reader r(0);

    const size_t n = 1'000'000;
    const size_t iterations = 250;
    
    int* a = new int[n];

    std::cout << "# noise: ";
    std::cout.flush();
    rapl::Power p_noise;// = noise(r);
    std::cout << p_noise.total() << std::endl;;

    for(size_t it = 0; it < iterations; it++) {
    
        rapl::Power p_random;
        {
            auto t0 = time();
            auto e0 = r.read();
            
            std::random_device rnd;
            std::default_random_engine e(rnd());
            std::uniform_int_distribution<int> uniform_dist(1, n);
            for(size_t i = 0; i < n; i++) {
                a[i] = uniform_dist(e);
            }

            p_random = rapl::Power(r.read() - e0, time() - t0) - p_noise;
        }

        auto p_add = test_add(r, a, n) - p_noise;
        auto p_xor = test_xor(r, a, n) - p_noise;
        auto p_mul = test_mul(r, a, n) - p_noise;

        std::cout << "RESULT op=rnd it=" << (it+1) << " power=" << p_random.total() << std::endl;
        std::cout << "RESULT op=add it=" << (it+1) << " power=" << p_add.total() << std::endl;
        std::cout << "RESULT op=xor it=" << (it+1) << " power=" << p_xor.total() << std::endl;
        std::cout << "RESULT op=mul it=" << (it+1) << " power=" << p_mul.total() << std::endl;
    }

    delete[] a;
    return 0;
}
