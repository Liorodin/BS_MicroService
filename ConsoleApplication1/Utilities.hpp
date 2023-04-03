#pragma once
#ifndef UTILTIES_HPP
#define UTILTIES_HPP
#include <NIDAQmx.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

void writeOutput(TaskHandle taskHandle, int delay, int duration);

class Observer;

class Subject {
public:
    Subject();
    void attach(Observer* observer);
    void detach(Observer* observer);
    void notify();
    void set_state();
    virtual ~Subject() {}
private:
    std::vector<Observer*> observers_;
    std::string state_;
};

// Observer class
class Observer {
public:
    virtual ~Observer() {}
    virtual void update(Subject* subject) = 0;
};

class ConcreteObserver : public Observer {
public:
    ConcreteObserver(TaskHandle handler, int delay, int duration) : handler_(handler), delay_(delay), duration_(duration) {}
    void update(Subject* subject) override;
private:
    TaskHandle handler_;
    int delay_;
    int duration_;
};
#endif