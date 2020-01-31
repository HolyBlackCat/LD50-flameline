#pragma once

#include <functional>

#include "macros/generated.h"

/* `class CoTask` is basically a poor man's coroutine.
 *
 * Example usage:
 *
 *     CoTask t = CO_TASK(,{
 *         std::cout << "1\n";
 *         CO_YIELD
 *         std::cout << "2\n";
 *         CO_YIELD
 *         std::cout << "3\n";
 *     });
 *     // {} around the task body are optional.
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
 * In short, the thing works by constructing a lambda with a large switch inside, and placing CO_YIELD-separated pieces of code under separate `case`s.
 *
 * Since we use switch/case, you can't CO_YIELD from a nested function or a nested lambda.
 *
 * Also switch/case can't jump across initializations, and because of that, USAGE OF LOCAL VARIABLES IS VERY LIMITED.
 *
 * The rule is: you can't have CO_YIELD in the same scope (or nested in such scope) as a local variable below said variable.
 * The compiler won't enforce the rule for variables with trivial default constructors without initializers,
 * but keep in mind that such variables won't retain their values after resuming the task, so they shouldn't be used.
 *
 * If you want to store the state, you have to declare all the state variables at the beginning of the function, like so:
 *
 *     CoTask t = CO_TASK((i = 0,), // <- All variables are declared here. Don't forget the trailing comma.
 *     {
 *         std::cout << ++i << "\n";
 *         CO_YIELD
 *         std::cout << ++i << "\n";
 *         CO_YIELD
 *         std::cout << ++i << "\n";
 *     });
 *
 * The syntax here is the same as in a lambda capture list since we use lambdas under the hood, so you can have regular captures too.
 * The underlying lambda is always mutable.
 */


class CoTask
{
    std::function<bool()> func;
    bool done = 0;

  public:
    CoTask() {}
    CoTask(std::function<bool()> func) : func(std::move(func)), done(0) {}

    explicit operator bool() const
    {
        return bool(func);
    }

    bool operator()() // Returns 1 if the task has finished.
    {
        if (!func)
            return 1;
        done = func();
        return done;
    }

    bool finished() const
    {
        if (!func)
            return 1;
        return done;
    }
};


#define CO_TASK(vars, ...) MA_CALL(IMPL_CO_TASK, vars, (;__VA_ARGS__))
#define CO_YIELD )(

#define IMPL_CO_TASK(vars, body_seq) \
    ::CoTask([ MA_IF_NOT_EMPTY(MA_IDENTITY vars, vars) _co_task_state = 0]() mutable -> bool \
    { \
        switch (_co_task_state) \
        { \
            MA_SEQ_FOR_EACH(,IMPL_CO_TASK_CASE,body_seq) \
        } \
        return 1; \
    })

#define IMPL_CO_TASK_CASE(data, i, ...) do {_co_task_state = i; return 0; case i: ; } while (0) __VA_ARGS__
