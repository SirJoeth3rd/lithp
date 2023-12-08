//Standard library functions

#include "lithp_std_lib.hpp"

lval_sptr plus(lval_sptr b, lithp_engine* e) {
  int result = 0;
  while (b) {
    if (b->type == Int) {
      result += std::get<int>(b->data);
    }
    b = b->get_next();
  }
  return std::make_shared<lval>(lval(Int,result));
}

lval_sptr mul(lval_sptr b, lithp_engine* e) {
  int result = 1;
  while (b) {
    if (b->type == Int) {
      result *= std::get<int>(b->data);
    }
    b = b->get_next();
  }
  return std::make_shared<lval>(lval(Int,result));
}

lval_sptr with(lval_sptr b, lithp_engine* e) {
  //expect shape with[[[symbol, expr]...],expr]
  lval_sptr args_lists = b->get_branch();
  lval_sptr expr = b->get_next();
  e->push_namespace();
  while (args_lists) {
    lval_sptr sym = args_lists->get_branch();
    lval_sptr expr = sym->get_next();
    e->eval(expr);
    e->set_symbol(std::get<string>(sym->data), expr);
    args_lists = args_lists->get_next();
  }
  e->eval(expr);
  e->pop_namespace();
  return expr;
}

class lfunc_store {
public:
  lval_sptr symbols;
  lval_sptr body;
  
  lval_sptr operator()(lval_sptr args, lithp_engine* engine) {
    //expect shape function([symbol,...], expr)
    //mathced with f(expr,...)
    lval_sptr syms = symbols;
    
    lval_sptr With = std::make_shared<lval>(lval(Lambda, with));
    With->is_macro = true;

    lval_sptr arg_lists = std::make_shared<lval>(lval(List));
    arg_lists->insert_branch(std::make_shared<lval>(lval(List)));
    lval_sptr alist = arg_lists->get_branch();
    
    while (syms) {
      alist->insert_branch(std::make_shared<lval>(syms->clone()));
      alist->get_branch()->insert_next(std::make_shared<lval>(args->clone()));
      alist->insert_next(std::make_shared<lval>(lval(List)));
      alist = alist->get_next();
      syms = syms->get_next();
      args = args->get_next();
    }

    //last alist should be removed
    if (!(alist->get_prev() == arg_lists)) {
      alist->remove_prev();
    }

    With->insert_branch(arg_lists);
    With->get_branch()->insert_next(body->clone_recurse()); //very inefficient

    engine->eval(With);

    return With;
  }

  lfunc_store(lval_sptr Symbols, lval_sptr Body) {
    symbols = Symbols->clone_recurse();
    body = Body->clone_recurse();
  }
};

lval_sptr function(lval_sptr h, lithp_engine* engine) {
  //expect shape function([s,...], expr)
  lfunc_store func_store = lfunc_store(h->get_branch(), h->get_next());
  return std::make_shared<lval>(lval(Lambda, func_store));
}

lval_sptr set(lval_sptr h, lithp_engine* engine) {
  //expect set(symbol, expr)
  lval_sptr expr = h->get_next();
  engine->eval(expr);
  engine->set_symbol(std::get<string>(h->data),expr);
  return std::make_unique<lval>(lval()); //just return nil
}

void load_all_functions(lithp_engine* engine) {
  lval_sptr Plus = std::make_shared<lval>(lval(Lambda,plus));
  lval_sptr Mul = std::make_shared<lval>(lval(Lambda,mul));
  lval_sptr With = std::make_shared<lval>(lval(Lambda, with));
  lval_sptr Function = std::make_shared<lval>(lval(Lambda, function));
  lval_sptr Set = std::make_shared<lval>(lval(Lambda, set));
  
  With->set_macro(true);
  Function->set_macro(true);
  Set->set_macro(true);

  engine->set_symbol("plus", Plus);
  engine->set_symbol("mul", Mul);
  engine->set_symbol("with", With);
  engine->set_symbol("function", Function);
  engine->set_symbol("set", Set);
}
