#include <cstdint>
#include <iostream>
#include <variant>

#include <stash/util/time.hpp>

class uint64_arithmetic {
public:
    virtual uint64_t v_op(uint64_t a, uint64_t b) const = 0;
};

class uint64_add : public uint64_arithmetic {
public:
    inline uint64_t op(uint64_t a, uint64_t b) const {
        return a + b;
    }

    virtual inline uint64_t v_op(uint64_t a, uint64_t b) const override {
        return a + b;
    }
};

class uint64_mul : public uint64_arithmetic {
public:
    inline uint64_t op(uint64_t a, uint64_t b) const {
        return a * b;
    }

    virtual inline uint64_t v_op(uint64_t a, uint64_t b) const override {
        return a * b;
    }
};

template<typename arithmetic_t>
inline uint64_t compute_template(arithmetic_t& x, uint64_t a, uint64_t b) {
    return x.op(a, b);
}

using op_variant = std::variant<uint64_add, uint64_mul>;

inline uint64_t compute_variant(const op_variant& x, uint64_t a, uint64_t b) {
    if(std::holds_alternative<uint64_add>(x)) {
        return std::get<uint64_add>(x).op(a, b);
    } else {
        return std::get<uint64_mul>(x).op(a, b);
    }
}

inline uint64_t compute_virtual(const uint64_arithmetic* x, uint64_t a, uint64_t b) {
    return x->v_op(a, b);
}

int main(int argc, char** argv) {
    using namespace stash;
    constexpr size_t num_ops = 10'000'000'000;
    
    // template
    {
        uint64_add add;
        uint64_mul mul;
        uint64_t result = 0;

        const auto t0 = time();
        for(size_t i = 0; i < num_ops; i++) {
            result = compute_template(add, result, i);
            result = compute_template(mul, result, i);
        }

        const auto dt = time() - t0;
        std::cout << "RESULT method=template time=" << dt << " check=" << result << std::endl;
    }

    // variant
    {
        op_variant add = uint64_add();
        op_variant mul = uint64_mul();
        uint64_t result = 0;

        const auto t0 = time();
        for(size_t i = 0; i < num_ops; i++) {
            result = compute_variant(add, result, i);
            result = compute_variant(mul, result, i);
        }

        const auto dt = time() - t0;
        std::cout << "RESULT method=variant time=" << dt << " check=" << result << std::endl;
    }
    
    // virtual
    {
        uint64_arithmetic* add = new uint64_add();
        uint64_arithmetic* mul = new uint64_mul();
        uint64_t result = 0;

        const auto t0 = time();
        for(size_t i = 0; i < num_ops; i++) {
            result = compute_virtual(add, result, i);
            result = compute_virtual(mul, result, i);
        }

        const auto dt = time() - t0;
        std::cout << "RESULT method=virtual time=" << dt << " check=" << result << std::endl;

        delete add;
        delete mul;
    }
    
    return 0;
}
