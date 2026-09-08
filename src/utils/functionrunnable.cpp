#include "functionrunnable.h"

FunctionRunnable::FunctionRunnable(std::function<void ()> func)
    :func(std::move(func))
{
}

void FunctionRunnable::run()
{
    if(this->func)
        func();
}
