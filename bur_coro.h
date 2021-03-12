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
    if (!m_last_result_read) {
      throw bur_coro_exception();
    }
    if (m_last_result_set) {
      throw bur_coro_exception();
    }
    if (!is_executing()) {
      throw bur_coro_exception();
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
    if (!m_last_result_set) {
      throw bur_coro_exception();
    }
    if (m_last_result_read) {
      throw bur_coro_exception();
    }
    if (!is_exemaning()) {
      throw bur_coro_exception();
    }
    m_last_result_read = true;
    m_last_result_set = false;
    return m_last_result;
  }
  void set_execution_position(int pos) {
    m_execution_position = pos;
  }
  int get_execution_position() const {
    return m_execution_position;
  }
  operator int() {
    return get_execution_position();
  }
  enum exection_state {
      execution_state_none = 0
    , execution_state_executing = 1
    , execution_state_examinate_result = 2
    , execution_state_finished = 3
    , execution_state_max = 4
  };
  void next_execution_state()
  {
    if (m_execution_state < execution_state_executing) {
      throw bur_coro_exception();
    }
    else if (m_execution_state >= execution_state_finished) {
      throw bur_coro_exception();
    }
    ++m_execution_state;
  }
  void set_execution_state_execution() {
    m_execution_state = execution_state_executing;
  }
  void set_execution_state_examination() {
    if (m_execution_state != execution_state_executing) {
      throw bur_coro_exception();
    }
    m_execution_state = execution_state_examinate_result;
  }
  void set_execution_state_finished() {
    if (m_execution_state != execution_state_examinate_result) {
      throw bur_coro_exception();
    }
    m_execution_state = execution_state_finished;
  }
  bool is_executing() const
  {
    return m_execution_state == execution_state_executing;
  }
  bool is_exemaning() const
  {
    return m_execution_state == execution_state_examinate_result;
  }
  bool is_finished() const
  {
    return m_execution_state == execution_state_finished;
  }
private:
  UINT m_last_result = ERR_OK;
  bool m_last_result_read = true;
  bool m_last_result_set = false;

  int m_execution_position = -1;
  int m_execution_state = execution_state_executing;
};


#define REENTER(cor)         \
switch(auto& cor_val = cor)  \
case -1:                     \
if (cor_val.is_finished()) { \
bailout:                     \
break;                       \
} else

#define YIELD_IMPL(n)                                       \
{                                                           \
  cor_val.set_execution_position(n);                        \
}                                                           \
case n:                                                     \
for(;;cor_val.next_execution_state())                       \
  if (cor_val.is_exemaning()) {                             \
    if (cor_val.read_last_result() == ERR_FUB_BUSY) {       \
      cor_val.set_execution_state_execution();              \
      break;                                                \
    }                                                       \
  }                                                         \
  else if (cor_val.is_finished()) {                         \
    goto bailout;                                           \
  }                                                         \
  else

#define YIELD YIELD_IMPL(__COUNTER__ + 1)



#endif

