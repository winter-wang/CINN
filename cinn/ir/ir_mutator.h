/**
 * This file implements the IRMutator as the base interface to mutate the IR.
 */
#pragma once

#include "cinn/ir/ir.h"
#include "cinn/ir/ir_visitor.h"

namespace cinn {
namespace ir {

//! T might be Expr* or const Expr*
template <typename T = Expr *>
class IRMutator : public IRVisitorBase<void, T> {
 public:
  void Visit(const Expr *expr, T op) override;

#define __(op__) void Visit(const op__ *op, T expr) override;
  NODETY_FORALL(__)
#undef __
};

template <typename T>
void IRMutator<T>::Visit(const Expr *expr, T op) {
  IRVisitorBase<void, T>::Visit(expr, op);
}

#define UNARY_OP_IMPL(op__)                                \
  template <typename T>                                    \
  void IRMutator<T>::Visit(const op__ *expr, T op) {       \
    auto *node = op->template As<op__>();                  \
    IRVisitorBase<void, T>::Visit(&node->v(), &node->v()); \
  }

#define BINARY_OP_IMPL(op__)                               \
  template <typename T>                                    \
  void IRMutator<T>::Visit(const op__ *expr, T op) {       \
    auto *node = op->template As<op__>();                  \
    IRVisitorBase<void, T>::Visit(&node->a(), &node->a()); \
    IRVisitorBase<void, T>::Visit(&node->b(), &node->b()); \
  }

NODETY_UNARY_OP_FOR_EACH(UNARY_OP_IMPL)
NODETY_BINARY_OP_FOR_EACH(BINARY_OP_IMPL)

#undef UNARY_OP_IMPL
#undef BINARY_OP_IMPL

template <typename T>
void IRMutator<T>::Visit(const IntImm *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const UIntImm *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const FloatImm *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const StringImm *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const Cast *expr, T op) {
  auto *node = op->template As<Cast>();
  Visit(&node->v(), &node->v());
}
template <typename T>
void IRMutator<T>::Visit(const For *expr, T op) {
  auto *node = op->template As<For>();
  IRVisitorBase<void, T>::Visit(&node->min, &node->min);
  IRVisitorBase<void, T>::Visit(&node->extent, &node->extent);
  IRVisitorBase<void, T>::Visit(&node->body, &node->body);
}
template <typename T>
void IRMutator<T>::Visit(const PolyFor *expr, T op) {
  auto *node = op->template As<PolyFor>();
  IRVisitorBase<void, T>::Visit(&node->body, &node->body);

  IRVisitorBase<void, T>::Visit(&node->inc, &node->inc);
}
template <typename T>
void IRMutator<T>::Visit(const Select *expr, T op) {
  auto *node = op->template As<Select>();
  IRVisitorBase<void, T>::Visit(&node->condition, &node->condition);
  IRVisitorBase<void, T>::Visit(&node->true_value, &node->true_value);
  IRVisitorBase<void, T>::Visit(&node->false_value, &node->false_value);
}
template <typename T>
void IRMutator<T>::Visit(const IfThenElse *expr, T op) {
  auto *node = op->template As<IfThenElse>();
  IRVisitorBase<void, T>::Visit(&node->condition, &node->condition);
  IRVisitorBase<void, T>::Visit(&node->true_case, &node->true_case);
  if (node->false_case.defined()) IRVisitorBase<void, T>::Visit(&node->false_case, &node->false_case);
}
template <typename T>
void IRMutator<T>::Visit(const Block *expr, T op) {
  auto *node = op->template As<Block>();
  for (auto &expr : node->stmts) {
    IRVisitorBase<void, T>::Visit(&expr, &expr);
  }
}
template <typename T>
void IRMutator<T>::Visit(const Call *expr, T op) {
  auto *node = op->template As<Call>();
  for (auto &expr : node->read_args) {
    IRVisitorBase<void, T>::Visit(&expr, &expr);
  }
  for (auto &expr : node->write_args) {
    IRVisitorBase<void, T>::Visit(&expr, &expr);
  }
}
template <typename T>
void IRMutator<T>::Visit(const _Module_ *expr, T op) {
  auto *node = op->template As<_Module_>();
  for (auto &func : node->functions) {
    IRVisitorBase<void, T>::Visit(&func, &func);
  }
  for (auto &func : node->buffers) {
    IRVisitorBase<void, T>::Visit(&func, &func);
  }
  for (auto &expr : node->submodules) {
    IRVisitorBase<void, T>::Visit(&expr, &expr);
  }
}
template <typename T>
void IRMutator<T>::Visit(const _Var_ *expr, T op) {
  auto *node = op->template As<ir::_Var_>();
  if (expr->is_reduce_axis) {
    IRVisitorBase<void, T>::Visit(&node->lower_bound, &node->lower_bound);
    IRVisitorBase<void, T>::Visit(&node->upper_bound, &node->upper_bound);
  }
}
template <typename T>
void IRMutator<T>::Visit(const Load *expr, T op) {
  auto *node = op->template As<Load>();
  for (auto &idx : node->indices) IRVisitorBase<void, T>::Visit(&idx, &idx);
}
template <typename T>
void IRMutator<T>::Visit(const Store *expr, T op) {
  auto *node = op->template As<Store>();
  IRVisitorBase<void, T>::Visit(&node->value, &node->value);
  IRVisitorBase<void, T>::Visit(&node->tensor, &node->tensor);
  for (auto &idx : node->indices) IRVisitorBase<void, T>::Visit(&idx, &idx);
}
template <typename T>
void IRMutator<T>::Visit(const Alloc *expr, T op) {
  auto *node = op->template As<Alloc>();
  for (auto &e : node->extents) {
    IRVisitorBase<void, T>::Visit(&e, &e);
  }

  if (node->condition.defined()) IRVisitorBase<void, T>::Visit(&node->condition, &node->condition);
  if (node->body.defined()) {
    Expr body(node->body);
    IRVisitorBase<void, T>::Visit(&node->body, &body);
  }
}
template <typename T>
void IRMutator<T>::Visit(const Free *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const _Range_ *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const _IterVar_ *expr, T op) {}
template <typename T>
void IRMutator<T>::Visit(const _Buffer_ *expr, T op) {
  auto *node = op->template As<_Buffer_>();

  for (auto &e : node->shape) {
    IRVisitorBase<void, T>::Visit(&e, &e);
  }
  for (auto &e : node->strides) {
    IRVisitorBase<void, T>::Visit(&e, &e);
  }
  IRVisitorBase<void, T>::Visit(&node->elem_offset, &node->elem_offset);
}
template <typename T>
void IRMutator<T>::Visit(const _Tensor_ *expr, T op) {
  auto *node = op->template As<_Tensor_>();

  for (auto &e : node->shape) {
    IRVisitorBase<void, T>::Visit(&e, &e);
  }
}
template <typename T>
void IRMutator<T>::Visit(const _LoweredFunc_ *expr, T op) {
  auto *node = op->template As<_LoweredFunc_>();
  IRVisitorBase<void, T>::Visit(&node->body, &node->body);
}
template <typename T>
void IRMutator<T>::Visit(const Let *expr, T op) {
  auto *node = op->template As<Let>();
  IRVisitorBase<void, T>::Visit(&node->symbol, &node->symbol);
  if (node->body.defined()) IRVisitorBase<void, T>::Visit(&node->body, &node->body);
}
template <typename T>
void IRMutator<T>::Visit(const Reduce *expr, T op) {
  auto *node = op->template As<Reduce>();
  IRVisitorBase<void, T>::Visit(&node->init, &node->init);
  IRVisitorBase<void, T>::Visit(&node->body, &node->body);
}

template <typename T>
void IRMutator<T>::Visit(const Ramp *expr, T op) {
  auto *node = op->template As<Ramp>();
  IRVisitorBase<void, T>::Visit(&node->base, &node->base);
  IRVisitorBase<void, T>::Visit(&node->stride, &node->stride);
}

template <typename T>
void IRMutator<T>::Visit(const Broadcast *expr, T op) {
  auto *node = op->template As<Broadcast>();
  IRVisitorBase<void, T>::Visit(&node->value, &node->value);
}

template <typename T>
void IRMutator<T>::Visit(const FracOp *expr, T op) {
  auto *node = op->template As<FracOp>();
  IRVisitorBase<void, T>::Visit(&node->a(), &node->a());
  IRVisitorBase<void, T>::Visit(&node->b(), &node->b());
}

template <typename T>
void IRMutator<T>::Visit(const Power *expr, T op) {
  auto *node = op->template As<Power>();
  IRVisitorBase<void, T>::Visit(&node->a(), &node->a());
  IRVisitorBase<void, T>::Visit(&node->b(), &node->b());
}

template <typename T>
void IRMutator<T>::Visit(const Product *expr, T op) {
  auto *node = op->template As<Product>();
  for (auto &x : node->operands()) {
    IRVisitorBase<void, T>::Visit(&x, &x);
  }
}

template <typename T>
void IRMutator<T>::Visit(const Sum *expr, T op) {
  auto *node = op->template As<Sum>();
  for (auto &x : node->operands()) {
    IRVisitorBase<void, T>::Visit(&x, &x);
  }
}

}  // namespace ir
}  // namespace cinn
