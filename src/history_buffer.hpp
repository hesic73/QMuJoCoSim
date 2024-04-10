#ifndef QMUJOCOSIM_HISTORY_BUFFER_HPP
#define QMUJOCOSIM_HISTORY_BUFFER_HPP


#include <vector>
#include <cstdint>
#include <atomic>
#include <cassert>

#include "mujoco/mujoco.h"

class HistoryBuffer {
public:
    explicit HistoryBuffer() {}

    void initialize(mjModel *m, mjData *d) {
        constexpr int kMaxHistoryBytes = static_cast<int>(1e8);
        constexpr int maxHistoryLength = 2000;

        state_size_ = mj_stateSize(m, mjSTATE_INTEGRATION);
        int state_bytes = state_size_ * static_cast<int>(sizeof(mjtNum));
        int history_length = std::min(std::numeric_limits<int>::max() / state_bytes, maxHistoryLength);
        int history_bytes = std::min(state_bytes * history_length, kMaxHistoryBytes);
        nhistory_ = history_bytes / state_bytes;

        // allocate history buffer, reset cursor and UI slider
        history_.clear();
        history_.resize(nhistory_ * state_size_);
        history_cursor_ = 0;
        scrub_index = 0;

        // fill buffer with initial state
        mj_getState(m, d, history_.data(), mjSTATE_INTEGRATION);
        for (int i = 1; i < nhistory_; ++i) {
            mju_copy(&history_[i * state_size_], history_.data(), state_size_);
        }
    }


    void loadScrubState(mjModel *m, mjData *d) {
        // get index into circular buffer
        int i = (scrub_index + history_cursor_) % nhistory_;
        i = (i + nhistory_) % nhistory_;

        // load state
        mjtNum *state = &history_[i * state_size_];
        mj_setState(m, d, state, mjSTATE_INTEGRATION);

        // call forward dynamics
        mj_forward(m, d);
    }

    void addToHistory(mjModel *m, mjData *d) {
        if (history_.empty()) {
            return;
        }

        // circular increment of cursor
        history_cursor_ = (history_cursor_ + 1) % nhistory_;

        // add state at cursor
        mjtNum *state = &history_[state_size_ * history_cursor_];
        mj_getState(m, d, state, mjSTATE_INTEGRATION);
    }


    void setScrubIndex(int scrubIndex) {
        scrub_index = scrubIndex;
    }

    int getScrubIndex() const {
        return scrub_index;
    }


    int size() const {
        return nhistory_;
    }

private:

    std::vector<mjtNum> history_;

    int state_size_ = 0;      // number of mjtNums in a history buffer state
    int nhistory_ = 0;        // number of states saved in history buffer
    int history_cursor_ = 0;  // cursor pointing at last saved state

    std::atomic_int scrub_index = 0;// index of history-scrubber slider
};

#endif //QMUJOCOSIM_HISTORY_BUFFER_HPP
