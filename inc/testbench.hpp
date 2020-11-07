#pragma once

template<class T>
class TestBench
{
public:
    T* dut; // The device being tested
    void tick();
    TestBench();
};

template<class T>
void TestBench<T>::tick()
{
    dut->eval();
}

template<class T>
TestBench<T>::TestBench()
{
    dut = new T; // Assign the model the testbench manipulates
}
