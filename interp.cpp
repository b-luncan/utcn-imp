// This file is part of the IMP project.

#include "interp.h"
#include "program.h"

#include <iostream>
#include <limits>



// -----------------------------------------------------------------------------
void Interp::Run()
{
  for (;;) {
    auto op = prog_.Read<Opcode>(pc_);
    switch (op) {
      case Opcode::PUSH_FUNC: {
        Push(prog_.Read<size_t>(pc_));
        continue;
      }
      case Opcode::PUSH_PROTO: {
        Push(prog_.Read<RuntimeFn>(pc_));
        continue;
      }
      case Opcode::PEEK: {
        auto idx = prog_.Read<unsigned>(pc_);
        Push(*(stack_.rbegin() + idx));
        continue;
      }
      case Opcode::POP: {
        Pop();
        continue;
      }
      case Opcode::CALL: {
        auto callee = Pop();
        switch (callee.Kind) {
          case Value::Kind::PROTO: {
            (*callee.Val.Proto) (*this);
            continue;
          }
          case Value::Kind::ADDR: {
            Push(pc_);
            pc_ = callee.Val.Addr;
            continue;
          }
          case Value::Kind::INT: {
            throw RuntimeError("cannot call integer");
          }
        }
        continue;
      }
      case Opcode::ADD: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (((rhs > 0) && (lhs > std::numeric_limits<int64_t>::max() - rhs)) ||
           ((rhs < 0) && (lhs < std::numeric_limits<int64_t>::min() - rhs)))
        {
          throw RuntimeError("add produced an invalid result");
        }
        Push(lhs + rhs);
        continue;
      }
      case Opcode::SUB: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (((rhs < 0) && (lhs > std::numeric_limits<int64_t>::max() + rhs)) ||
           ((rhs > 0) && (lhs < std::numeric_limits<int64_t>::min() + rhs)))
        {
          throw RuntimeError("add produced an invalid result");
        }
        Push(lhs - rhs);
        continue;
      }
      case Opcode::MULTIPLY: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (lhs > std::numeric_limits<int64_t>::max() / rhs)
        {
          throw RuntimeError("multiply produced an invalid result");
        }
        Push(lhs * rhs);
        continue;
      }
      case Opcode::DIVISION: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (lhs == 0)
        {
          throw RuntimeError("Invalid division with 0");
        }
        Push(lhs / rhs);
        continue;
      }
      case Opcode::MODULO: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        if (lhs == 0)
        {
          throw RuntimeError("Invalid modulo with 0");
        }
        Push(lhs % rhs);
        continue;
      }
      case Opcode::RET: {
        auto depth = prog_.Read<unsigned>(pc_);
        auto nargs = prog_.Read<unsigned>(pc_);
        auto v = Pop();
        stack_.resize(stack_.size() - depth);
        pc_ = PopAddr();
        stack_.resize(stack_.size() - nargs);
        Push(v);
        continue;
      }
      case Opcode::JUMP_FALSE: {
        auto cond = Pop();
        auto addr = prog_.Read<size_t>(pc_);
        if (!cond) {
          pc_ = addr;
        }
        continue;
      }
      case Opcode::JUMP: {
        pc_ = prog_.Read<size_t>(pc_);
        continue;
      }
      case Opcode::STOP: {
        return;
      }
      case Opcode::PUSH_INT: {
        auto value = prog_.Read<int64_t>(pc_);
        Push(value);
        continue;
      }
      case Opcode::EQUALITY: {
        auto rhs = PopInt();
        auto lhs = PopInt();
        Push(static_cast<int64_t>(lhs == rhs));
        continue;
      }
    }
  }
}
