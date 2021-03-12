#include "bur_coro.h"
#include "bur_std.h"

#include <iostream>
#include <cassert>

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

