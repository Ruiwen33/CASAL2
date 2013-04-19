/**
 * @file RandomNumberGenerator.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @version 1.0
 * @date 28/02/2013
 * @section LICENSE
 *
 * Copyright NIWA Science �2013 - www.niwa.co.nz
 *
 * $Date: 2008-03-04 16:33:32 +1300 (Tue, 04 Mar 2008) $
 */

// Headers
#include "RandomNumberGenerator.h"

#include "GlobalConfiguration/GlobalConfiguration.h"

// Namespaces
namespace isam {
namespace utilities {

/**
 * Default Constructor
 */
RandomNumberGenerator::RandomNumberGenerator() {
  generator_.seed(GlobalConfiguration::Instance()->random_seed());
}

/**
 * Destructor
 */
RandomNumberGenerator::~RandomNumberGenerator() {
}

/**
 * Singleton instance method
 * @return reference to singleton object
 */
RandomNumberGenerator& RandomNumberGenerator::Instance() {
  static RandomNumberGenerator instance;
  return instance;
}

/**
 * Get a random uniform between min and max
 *
 * @param min The smallest number to be generated (default 0.0)
 * @param max The largest number to be generated (default 1.0)
 * @return random number
 */
double RandomNumberGenerator::uniform(double min, double max) {
  boost::uniform_real<> uniform(min, max);
  boost::variate_generator<boost::mt19937_64&, boost::uniform_real<> > generator(generator_, uniform);

  return generator();
}

/**
 * Generate a normal distributed random number
 *
 * @param mean (default 0.0)
 * @param sigma (default 1.0)
 * @return random number
 */
double RandomNumberGenerator::normal(double mean, double sigma) {
  boost::normal_distribution<> normal(mean, sigma);
  boost::variate_generator<boost::mt19937_64&, boost::normal_distribution<> > generator(generator_, normal);

  return generator();
}

/**
 * Generate a log normal random number
 *
 * @param mean
 * @param cv
 * @return random number
 */
double RandomNumberGenerator::log_normal(double mean, double cv) {
  double log_sigma = sqrt(log(cv*cv + 1.0));
  double log_mean  = log(mean) - (log_sigma * log_sigma) / 2.0;

  double random_number = normal(log_mean, log_sigma);
  return std::exp(random_number);
}

/**
 * Generator a random binomial
 *
 * @param p
 * @param n
 * @return random number
 */
double RandomNumberGenerator::binomial(double p, double n) {
  double count = 0;
  for (unsigned i = 0; i < std::ceil(n); ++i) {
    double temp = uniform();
    if (temp <= p)
      count++;
  }

  return (count / std::ceil(n));
}

/**
 * Generate a chi square random number
 *
 * @param df Number of random numbers to generate
 * @return random number
 */
double RandomNumberGenerator::chi_square(unsigned df) {
  boost::normal_distribution<> normal(0.0, 1.0);
  double sum = 0.0;
  for (unsigned i = 0; i < df; ++i) {
    boost::variate_generator<boost::mt19937_64&, boost::normal_distribution<> > generator(generator_, normal);
    double random = generator();
    sum += random * random;
  }

  return sum;
}

} /* namespace utilities */
} /* namespace isam */
