#ifndef FUNCTIONRUNNABLE_H
#define FUNCTIONRUNNABLE_H

#include <QRunnable>

class FunctionRunnable : public QRunnable
{
public:
    explicit FunctionRunnable(std::function<void()> func);
    void run() override;

private:
    std::function<void()> func;
};

#endif // FUNCTIONRUNNABLE_H
