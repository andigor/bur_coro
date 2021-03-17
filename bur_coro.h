#ifndef BUR_CORO_H_
#define BUR_CORO_H_

#include "bur_std.h"

struct bur_coro_exception
{
};

class bur_coro
{
public:
  void set_last_result(UINT res) {
    // if (!m_last_result_read) {
    //   throw bur_coro_exception();
    // }
    // if (m_last_result_set) {
    //   throw bur_coro_exception();
    // }

    if (!is_executing_yield()) {
      if (!is_not_started()) {
        if (!is_yield_finished()) {
          throw bur_coro_exception();
        }
      }
    }

    m_last_result = res;
    m_last_result_set = true;
    m_last_result_read = false;
  }
  UINT get_last_result() const
  {
    return m_last_result;
  }
  UINT read_last_result() {
    // if (!m_last_result_set) {
    //   throw bur_coro_exception();
    // }
    // if (m_last_result_read) {
    //   throw bur_coro_exception();
    // }
    if (!is_examining()) {
      throw bur_coro_exception();
    }
    if (!has_valid_execution_position()) {
      throw bur_coro_exception();
    }
    m_last_result_read = true;
    m_last_result_set = false;
    return m_last_result;
  }
  int get_execution_position() const {
    return m_execution_position;
  }
  operator int() {
    return get_execution_position();
  }
  enum exection_state {
    execution_state_none = 0
    , execution_state_executing_yield = 1
    , execution_state_examine_result = 2
    , execution_state_yield_finished = 3
    , execution_state_max = 4
  };
  void next_execution_state()
  {
    if (m_execution_state < execution_state_executing_yield) {
      throw bur_coro_exception();
    }
    else if (m_execution_state >= execution_state_yield_finished) {
      throw bur_coro_exception();
    }
    if (!has_valid_execution_position()) {
      throw bur_coro_exception();
    }
    ++m_execution_state;
  }
  void set_execution_state_executing_yield(int pos) {
    if (!(m_execution_state == execution_state_none
      || m_execution_state == execution_state_examine_result
      || m_execution_state == execution_state_yield_finished)) {
      throw bur_coro_exception();
    }

    m_execution_state = execution_state_executing_yield;
    m_execution_position = pos;
  }
  void set_execution_state_examination() {
    if (m_execution_state != execution_state_executing_yield) {
      throw bur_coro_exception();
    }
    m_execution_state = execution_state_examine_result;
  }
  void set_execution_state_yield_finished() {
    if (m_execution_state != execution_state_examine_result) {
      throw bur_coro_exception();
    }
    m_execution_state = execution_state_yield_finished;

    mark_execution_position_as_finished();
  }
  bool is_not_started() const
  {
    return m_execution_state == execution_state_none;
  }
  bool is_executing_yield() const
  {
    return m_execution_state == execution_state_executing_yield;
  }
  bool is_examining() const
  {
    return m_execution_state == execution_state_examine_result;
  }
  bool is_yield_finished() const
  {
    return m_execution_state == execution_state_yield_finished;
  }
  bool is_last_result_set() const {
    return m_last_result_set;
  }
  bool is_last_result_read() const {
    return m_last_result_read;
  }
  bool is_execution_position_finished() const
  {
    return m_execution_position == -2;
  }
  void reset()
  {
    // reinitialize for the new execution
    *this = bur_coro();
  }
private:
  void mark_execution_position_as_finished() {
    if (!is_yield_finished()) {
      throw bur_coro_exception();
    }
    m_execution_position = -2;
  }
  bool has_valid_execution_position() const
  {
    return m_execution_position > 0;
  }

  UINT m_last_result = ERR_OK;
  bool m_last_result_read = true;
  bool m_last_result_set = false;

  int m_execution_position = -1;
  int m_execution_state = execution_state_none;
};


#define REENTER(cor)                                     \
switch(auto& cor_val = cor)                              \
case -1:                                                 \
case -2:                                                 \
if (cor_val.is_execution_position_finished()) {          \
    bailout:                                             \
break;                                                   \
} else

#define YIELD_IMPL(n)                                    \
{                                                        \
  cor_val.set_execution_state_executing_yield(n);        \
}                                                        \
case n:                                                  \
for(;;cor_val.next_execution_state())                    \
  if (cor_val.is_examining()) {                          \
    const auto last_result = cor_val.read_last_result(); \
    if (last_result == ERR_FUB_BUSY) {                   \
      cor_val.set_execution_state_executing_yield(n);    \
      goto bailout;                                      \
    }                                                    \
    else if (last_result == ERR_OK) {                    \
      cor_val.set_execution_state_yield_finished();      \
      break;                                             \
    }                                                    \
    else /*failure*/ {                                   \
      cor_val.set_execution_state_yield_finished();      \
      break;                                             \
    }                                                    \
  }                                                      \
  else

#define YIELD YIELD_IMPL(__COUNTER__ + 1)



#endif

