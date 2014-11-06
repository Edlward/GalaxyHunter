#ifndef UTILS_SEQUENCE_H
#define UTILS_SEQUENCE_H
#include <functional>

template<typename T, T defaultValue, typename check_operator = std::equal_to<T>>
class sequence {
public:
  typedef std::function<T()> run_function;
  typedef std::function<void(const T &, const std::string &)> on_error_f;
  struct run {
    run_function f;
    std::string label;
    T check;
    run(run_function f, const std::string &label = {}, T check = defaultValue) : f(f), label(label), check(check) {}
  };
  template <typename ...Args>
  sequence(const std::list<run> &runs) : runs(runs), _check_operator(check_operator{}) {}
  ~sequence() {
    for(auto r: runs) {
      T result = r.f();
      if(! _check_operator(result, r.check)) {
	_run_on_error(result, r.label);
	return;
      }
    };
  }
  sequence &on_error(on_error_f run_on_error) { _run_on_error = run_on_error; return *this; }
private:
  std::list<run> runs;
  on_error_f _run_on_error = [](const T&, const std::string&) {};
  check_operator _check_operator;
};

#endif