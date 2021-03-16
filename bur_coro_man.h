#ifndef BUR_CORO_MAN_H_
#define BUR_CORO_MAN_H_

struct resumable_test_class
{
  struct state {
    int move_state()
    {
      ++m_state;
      if (m_state % 3 == 0) {
        return 0;
      }
      return 65535; // busy
    }
    int m_state = 0;
  };
  void func()
  {
    // REENTER
    switch(m_pos)
      case -2:
      case -1:
        if (m_pos == -2) {
        bailout:
          break;
        }
        else

          /*REENTER()*/ {

          /*YIELD*/ {
              m_pos = 1;
      case 1:
        auto ret = m_state1.move_state();
        if (ret == 65535) {
          // busy
          goto bailout;
        }
        m_pos = -2;
            }

          /*YIELD*/ {
            m_pos = 2;
      case 2:
        auto ret = m_state1.move_state();
        if (ret == 65535) {
          //busy
          goto bailout;
        }
        m_pos = -2;
          }

        } /*REENTER END*/
  }
  int m_pos = -1;
  state m_state1;
  state m_state2;
};


inline void test_resumable_class_test()
{
  resumable_test_class t;

  assert(t.m_pos == -1);


  t.func();
  assert(t.m_pos == 1);

  t.func();
  assert(t.m_pos == 1);

  t.func();
  assert(t.m_pos == 2);

  t.func();
  assert(t.m_pos == 2);

  t.func();
  assert(t.m_pos == -2);

  t.m_state1.m_state = 9999;
  t.func();
  // not called again
  assert(t.m_state1.m_state == 9999);
  assert(t.m_pos == -2);
}

#endif
