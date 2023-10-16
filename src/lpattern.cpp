#include "lithp.hpp"
#include <memory>

//now here begins a new adventure
class lpattern {
public:
  
  lpattern (ltype tp) {
    type = tp;
  }

  lpattern () {
    any = true;
  }

  void operator[] (lpattern& pattern) {
    branch = std::make_unique<lpattern>(pattern);
  }

  std::shared_ptr<lpattern> operator>> (lpattern& pattern) {
    next = std::make_unique<lpattern>(pattern);
    pattern.prev = this->get_this();
    return std::make_unique<lpattern>(pattern);
  }

  std::shared_ptr<lpattern> operator|| (lpattern& pattern) {
    this->possibles.push_back(std::make_unique<lpattern>(pattern));
    return this->get_this();
  }

  std::shared_ptr<lpattern> get_this() {
    return std::make_unique<lpattern>(*this);
  }

  std::shared_ptr<lpattern> operator&() {
    return this->get_this();
  }
  
private:
  ltype type;
  bool any;
  std::shared_ptr<lpattern> next;
  std::shared_ptr<lpattern> prev;
  std::shared_ptr<lpattern> branch;
  std::vector<std::shared_ptr<lpattern>> possibles;
};
