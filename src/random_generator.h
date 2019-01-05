#ifndef RAYTRACING_RANDOM_GENERATOR_H
#define RAYTRACING_RANDOM_GENERATOR_H

#include <random>

/**
 * @File random_generator.h
 * @Brief holds the Random class, and some functions to fetch random number generators
 *
 * The Random class is based on the high-quality mt19937 (Mersenne Twister)
 * pseudo-random number generator, using the C++11 implementation
 *
 * The Random class is not intended to be created and used directly.
 * Its construction is costly; instead, use one of the getRandomGen() functions.
 */
class Random
{
private:
    std::mt19937 m_Generator; // mersenne twister generator

public:
    Random(unsigned seed = 123u);

    void Seed(unsigned seed);
    unsigned _next(void); // returns a raw 32-bit unbiased random integer

    int RandInt(int a, int b); // returns a random integer in [a..b] (a and b can be negative as well)
    float RandFloat(void); // return a floating-point number in [0..1)
    double RandDouble(void); // same as Randfloat(), but in double precision (using two _next() invocations)
    double Gaussian(double mean = 0.0, double sigma = 1.0); // return a random number in normal distribution
    void UnitDiscSample(double& x, double &y); // get a random point in the unit disc (x*x + y*y <= 1)
};

/// seed the whole array of random generators.
void InitRandom(unsigned seed);

/// fetch the idx-th random generator. There are at least 250 random generators, which are prepared and ready.
/// This function does not take any start-up time and should be very fast.
class Random& GetRandomGen(int idx);

/// fetch a fixed random generator, based on the calling thread's ID. I.e., within each thread, all calls to getRandomGen()
/// are guaranteed to return the same object; in the same time, different threads get different random generators
/// thus no locking is required, and no performance degradation can occur.
class Random& GetRandomGen(void);

void test_random();

#endif //RAYTRACING_RANDOM_GENERATOR_H
