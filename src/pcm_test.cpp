#include <unistd.h>

#include <pcm/cpucounters.h>

#include <stash/io/load_file.hpp>
#include <stash/util/time.hpp>

int main(int argc, char** argv) {
    // cmd
    if(argc < 2) {
        std::cerr << "usage: " << argv[0] << " <file>" << std::endl;
	return -1;
    }

    // init
    PCM* m = PCM::getInstance();

    const auto cpu_model = m->getCPUModel();
    if (!(m->hasPCICFGUncore()))
    {
        std::cerr << "unsupported processor model: " << cpu_model << std::endl;
        return -3;
    } 

    // before
    ServerUncorePowerState* state0 = new ServerUncorePowerState[m->getNumSockets()];
    for(int i = 0; i < m->getNumSockets(); ++i) {
        state0[i] = m->getServerUncorePowerState(i);
    }
    auto t0 = stash::time();

    // do stuff
    {
        uint64_t sum = 0;
        auto v = stash::io::load_file_lines_as_vector<uint64_t>(argv[1]);
        for(auto x : v) {
            sum += x;
	}
        std::cout << "sum=" << sum << std::endl;
    }
    //sleep(1);

    // after
    auto t1 = stash::time();
    ServerUncorePowerState* state1 = new ServerUncorePowerState[m->getNumSockets()];
    for(int i = 0; i < m->getNumSockets(); ++i) {
        state1[i] = m->getServerUncorePowerState(i);
    }

    // log
    std::cout << "time=" << (t1-t0) << std::endl;
    for(int i = 0; i < m->getNumSockets(); ++i) {
        const auto cpu = getConsumedJoules(state0[i], state1[i]);
        const auto ram = getDRAMConsumedJoules(state0[i], state1[i]);
        std::cout << "socket " << i << ": cpu_joules=" << cpu << ", ram_joules=" << ram << std::endl;
    }

    // clean up
    delete[] state0;
    delete[] state1;
    return 0;
}

