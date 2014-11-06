#ifndef UTILS_SEQUENCE_H
#define UTILS_SEQUENCE_H
#include <memory>

template<typename T, T defaultValue>
class sequence {
public:
  typedef std::function<T()> run_function;
  struct run {
    run_function f;
    T check;
    run(run_function f, T check = defaultValue) : f(f), check(check) {}
  };
  template <typename ...Args>
  sequence(const std::list<run> &runs) : runs(runs) {}
  ~sequence() {
    for(auto r: runs) {
      T result = r.f();
      if(result != r.check) {
	_run_on_error(result);
	return;
      }
    };
  }
  sequence &run_on_error(std::function<void(T)> run_on_error) { _run_on_error = run_on_error; return *this; }
private:
  std::list<run> runs;
  std::function<void(T)> _run_on_error = [](T){};
};

#endif