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
  bur_coro& get_coro()
  {
    return m_bur_coro;
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

struct resumable_with_resumable_lambda : public bur_coro
{
  resumable_with_resumable_lambda(int max_val)
    :m_inc(m_nested_coro, max_val)
  {

  }

  void operator()()
  {
    REENTER(*this) {
      YIELD [](auto& out_coro, auto& inc){
        REENTER(inc.get_coro()) {
          out_coro.set_last_result(ERR_FUB_BUSY);
          YIELD inc.increase_number();
          out_coro.set_last_result(inc.get_coro().get_last_result());
        }
      }(*this, m_inc);
    }
  }

  bur_coro m_nested_coro;
  bur_coro_number_increaser m_inc;
};

void test_resumable_with_resumable_lambda()
{
  resumable_with_resumable_lambda r(3);

  r();
  assert(r.m_inc.get_number() == 1);
  r();
  assert(r.m_inc.get_number() == 2);
  r();
  assert(r.m_inc.get_number() == 3);
  r();
  assert(r.m_inc.get_number() == 3);
}
#include "bur_coro_man.h"

int main()
{
  resumable_number_increaser incr(2);
  incr();
  assert(incr.get_number() == 1);
  assert(incr.get_number() == 1);
  incr();
  assert(incr.get_number() == 2);
  assert(incr.get_number() == 2);
  incr();
  assert(incr.get_number() == 2);
  assert(incr.get_number() == 2);

  test_resumable_class_test();

  test_resumable_with_resumable_lambda();
}

