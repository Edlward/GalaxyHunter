#ifndef UTILS_SCOPE_H
#define UTILS_SCOPE_H

class scope {
public:
  scope(std::function<void()> f) : f(f) {}
  ~scope() { f(); }
private:
  std::function<void()> f;
};

#endif
