#ifndef RANDOM_HPP_INCLUDED
#define RANDOM_HPP_INCLUDED
/*
 * C S O U N D
 *
 * L I C E N S E
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Platform.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include "Node.hpp"
%}
#else
#include "Node.hpp"
#include <random>
#include <cmath>
#include <cstdint>
#endif

namespace csound
{

/**
 * A random value will be sampled from the specified distribution,
 * translated and scaled as specified,
 * and set in the specified row and column of the local coordinates.
 * The resulting matrix will be used in place of the local coordinates
 * when traversing the music graph.
 * If eventCount is greater than zero, a new event will be created
 * for each of eventCount samples,
 * which will be transformed by the newly sampled local coordinates.
 */
class SILENCE_PUBLIC Random :
    public Node
{
protected:
#if !defined(SWIG)
    void *generator_;
    std::uniform_int_distribution<std::int32_t> uniform_smallint_generator;
    std::uniform_int_distribution<std::int64_t> uniform_int_generator;
    std::uniform_real_distribution<> uniform_real_generator;
    std::bernoulli_distribution bernoulli_distribution_generator;
    std::geometric_distribution<> geometric_distribution_generator;
    std::exponential_distribution<> exponential_distribution_generator;
    std::normal_distribution<> normal_distribution_generator;
    std::lognormal_distribution<> lognormal_distribution_generator;
public:
    static std::mt19937 mersenneTwister;
#endif
public:
    std::string distribution;
    int row;
    int column;
    int eventCount;
    bool incrementTime;
    double minimum;
    double maximum;
    double q;
    double a;
    double b;
    double c;
    double Lambda;
    double mean;
    double sigma;
    Random();
    virtual ~Random();
    virtual double sample();
    virtual Eigen::MatrixXd getRandomCoordinates();
    virtual void createDistribution(std::string distribution);
    virtual void generate(Score &score);
    virtual void transform(Score &score);
    static void seed(int s);
};

}

#endif

