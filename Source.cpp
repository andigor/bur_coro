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

    m_last_result = res;
    m_last_result_set = true;
  }
  UINT read_last_result() {
    if (!m_last_result_set) {
      throw bur_coro_exception();
    }
    if (m_last_result_read) {
      throw bur_coro_exception();
    }
    m_last_result_read = true;
    return m_last_result;
  }
  void set_execution_position(int pos) {
    m_execution_position = pos;
  }
  int get_execution_position() const {
    return m_execution_position;
  }
  enum exection_state {
      execution_state_none = 0
    , execution_state_executing = 1
    , exection_state_examinate_result = 2
  };
  void reset_execution_state() {
    m_execution_state = execution_state_executing;
  }
  void go_to_result_examination() {
    if (m_execution_state != execution_state_executing) {
      throw bur_coro_exception();
    }
    m_execution_state = exection_state_examinate_result;
  }
  bool is_executing() const
  {
    return m_execution_state == execution_state_executing;
  }
  bool is_exemaning() const
  {
    return m_execution_state == exection_state_examinate_result;
  }
private:
  UINT m_last_result = ERR_OK;
  bool m_last_result_read = true;
  bool m_last_result_set = false;

  int m_execution_position = -1;
  int m_execution_state = execution_state_none;
};


int bur_func_1(int& arg)
{
  ++arg;
  return 0; // ready
}

int bur_func_2(int& arg)
{
  ++arg;
  if (arg < 10) {
    return 65535; // busy
  }
  return 0; // ready
}

#define REENTER() \
switch(m_pos) \
case -1:      \
if (0) {      \
bailout:      \
break;        \
} else


#define YIELD_IMPL(n) \
{\
m_pos = n;\
}\
case n: \
for(m_iter_num = 0;;++m_iter_num) \
if (m_iter_num == 2) { \
goto bailout;\
}\
else if (m_iter_num == 1) {\
if (m_status == 0) {\
break;\
}\
}\
else m_status =



#define YIELD YIELD_IMPL(__COUNTER__ + 1)
#if 1
struct bur_task : bur_coro{
  int operator()()
  {
    REENTER() {
      YIELD bur_func_1(a); 
      std::cout << "after first: " << a << std::endl;
      a += 150;
      YIELD bur_func_2(a);
      std::cout << "after second : " << a << std::endl;
    }
    return 0;
  }
  int a = 0;
};


int main()
{
  bur_task tt;
  tt();
  std::cout << "after first call: " << std::endl;
  tt();
  std::cout << tt.a << std::endl;
}
#endif

#if 0
void func()
{
	int m_pos = -1;
	//int m_iter_num;
  //int m_status;
  int a = 1;

  struct bur_task : bur_coro {
    int operator()()
    {
    switch (m_pos) 
    case -1: 
      if (0) 
      { 
      bailout: 
        break; 
      }
      else {
        {m_pos = 0 + 1; }
        case 0 + 1: 
          for (m_iter_num = 0;; ++m_iter_num) 
            if (m_iter_num == 2) { goto bailout; }
            else if (m_iter_num == 1) { 
              if (m_status != 0) { break; } 
            }
            else m_status = bur_func_1(a);
            a = 150;
            {m_pos = 1 + 1; }
            case 1 + 1: 
          for (m_iter_num = 0;; ++m_iter_num) 
            if (m_iter_num == 2) { goto bailout; }
            else if (m_iter_num == 1) { if (m_status != 0) { break; } }
            else m_status = bur_func_2(a);
      }
      return 0;
    }
    int a = 0;
  };

  bur_task t;
  t();
  t();
}
int main()
{
  func();
}
#endif
