#include <iostream>
#include <cassert>

using UINT = std::uint16_t;

const UINT ERR_FUB_BUSY = 65535;
const UINT ERR_OK = 0;

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
  };
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
for(;;cor_val.set_execution_state_examination())            \
  if (cor_val.is_exemaning()) {                             \
    if (cor_val.read_last_result() == ERR_OK) {             \
      cor_val.set_execution_state_finished();               \
      goto bailout;                                         \
    }                                                       \
    else {                                                  \
      cor_val.set_execution_state_execution();              \
      break;                                                \
    }                                                       \
  }                                                         \
  else

#define YIELD YIELD_IMPL(__COUNTER__ + 1)

class bur_coro_number_increaser
{
public:
  bur_coro_number_increaser(bur_coro& coro, int max_num)
    : m_bur_coro(coro)
    , m_max_number(max_num)

  {
  }

  void increase_number()
  {
    if (m_number >= m_max_number) {
      m_bur_coro.set_last_result(ERR_OK);
      return;
    }
    ++m_number;
    if (m_number >= m_max_number) {
      m_bur_coro.set_last_result(ERR_OK);
      return;
    }
    m_bur_coro.set_last_result(ERR_FUB_BUSY);
  }

  int get_number() const
  {
    return m_number;
  }

  void reset()
  {
    m_number = 0;
  }
private:
  bur_coro& m_bur_coro;
  int m_number = 0;
  const int m_max_number = 0;
};

class resumable_number_increaser : public bur_coro
{
public:
  resumable_number_increaser(int max_val)
    :m_inc(*this, max_val)
  {
  }

  void operator()()
  {
    REENTER(*this) {
      YIELD m_inc.increase_number();
    }
  }
  int get_number() const
  {
    return m_inc.get_number();
  }
private:
  bur_coro_number_increaser m_inc;
};


int main()
{
  resumable_number_increaser incr(100);
  incr();
  assert(incr.get_number() == 1);
  assert(incr.get_number() == 1);
  incr();
  assert(incr.get_number() == 2);
  assert(incr.get_number() == 2);
}

