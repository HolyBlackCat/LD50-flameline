#pragma once

#include <functional>

#include "macro.h"

/* `class Task` is basically a poor man's coroutine.
 *
 * Example usage:
 *
 *     Task t = TASK((),{
 *         std::cout << "1\n";
 *         YIELD
 *         std::cout << "2\n";
 *         YIELD
 *         std::cout << "3\n";
 *     });
 *
 *     while (!t.finished())
 *     {
 *         std::cout << "Calling task...\n";
 *         t();
 *     }
 *
 *     std::cout << "Finished!\n";
 *
 * It prints:
 *
 *     Calling task...
 *     1
 *     Calling task...
 *     2
 *     Calling task...
 *     3
 *     Finished!
 *
 * In short, the thing works by constructing a lambda and replacing `YIELD` with `{ return x; goto_label_y: ; }`.
 *
 * Since we use `goto`, you can't YIELD from a nested function or a nested lambda.
 *
 * Also goto can't jump across initializations, and because of that, USAGE OF LOCAL VARIABLES IS VERY LIMITED.
 *
 * The rule is: you can't have YIELD in the same scope (or nested in such scope) as a local variable below said variable.
 * The compiler won't enforce the rule for variables with trivial default constructors without initializers,
 * but keep in mind that such variables won't retain their values after resuming the task, so they shouldn't be used.
 *
 * If you want to store the state, you have to declare all the state variables at the beginning of the function, like so:
 *
 *     Task t = TASK((i = 0,), // <- All variables are declared here. Don't forget the trailing comma.
 *     {
 *         std::cout << ++i << "\n";
 *         YIELD
 *         std::cout << ++i << "\n";
 *         YIELD
 *         std::cout << ++i << "\n";
 *     });
 *
 * The syntax here is the same as in a lambda capture list since we use lambdas under the hood, so you can have regular captures too.
 * The underlying lambda is always mutable.
 */


class Task
{
    std::function<bool()> func;
    bool done = 0;

  public:
    Task() {}
    Task(std::function<bool()> func) : func(std::move(func)) {}

    explicit operator bool() const
    {
        return bool(func);
    }

    bool operator()() // Returns 1 if the task has finished.
    {
        if (!func)
            return 1;
        done = func() == 0;
        return done;
    }

    bool finished() const
    {
        if (!func)
            return 1;
        return done;
    }
};


#define TASK(vars, ...) MA_CALL(IMPL_TASK, vars, (__VA_ARGS__))
#define YIELD )(

#define IMPL_TASK(vars, body_seq) \
    ::Task([MA_IDENTITY vars _label_index = -1]() mutable -> bool \
    { \
        switch (_label_index) \
        { \
            MA_SEQ_FOR_EACH(IMPL_TASK_CASE, MA_NULL, , body_seq) \
        } \
        MA_SEQ_FOR_EACH(IMPL_TASK_CODE, MA_NULL, , body_seq) \
        return 0; \
    })

#define IMPL_TASK_CASE(i, data, ...) case i: goto _task_point_##i;
#define IMPL_TASK_CODE(i, data, ...) __VA_ARGS__ {_label_index = i; return i; _task_point_##i: ;}
