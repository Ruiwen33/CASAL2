/**
 * @file RandomDraw.cpp
 * @author Craig Marsh
 * @github https://github.com/Zaita
 * @date 2/02/2016
 * @section LICENSE
 *
 * Copyright NIWA Science �2014 - www.niwa.co.nz
 *
 */

// headers
#include "RandomDraw.h"

#include "Utilities/Map.h"
#include "Utilities/RandomNumberGenerator.h"
#include "Model/Objects.h"
#include "Estimates/Manager.h"


// namespaces
namespace niwa {
namespace timevarying {

/**
 * Default constructor
 */
RandomDraw::RandomDraw(Model* model) : TimeVarying(model) {
  parameters_.Bind<Double>(PARAM_MEAN, &mu_, "Mean", "", 0);
  parameters_.Bind<Double>(PARAM_SIGMA, &sigma_, "Standard deviation", "", 1);
  parameters_.Bind<string>(PARAM_DISTRIBUTION, &distribution_, "distribution", "", PARAM_NORMAL);

  RegisterAsEstimable(PARAM_MEAN, &mu_);
  RegisterAsEstimable(PARAM_SIGMA, &sigma_);
}

/**
 *
 */
void RandomDraw::DoValidate() {
  if (distribution_ != PARAM_NORMAL)
    LOG_ERROR() << "Random Walk can only can draw from a normal distribution currently";
}

/**
 *
 */
void RandomDraw::DoBuild() {
  Estimate* estimate = model_->managers().estimate()->GetEstimate(parameter_);
  if (estimate) {
    LOG_FINEST() << "Found an @estimate block for " << parameter_;
    upper_bound_ = estimate->upper_bound();
    lower_bound_ = estimate->lower_bound();
  }

}

/**
 *
 */
void RandomDraw::DoReset() {

}

/**
 *
 */
void RandomDraw::DoUpdate() {
  string error = "";
  value_ = model_->objects().GetEstimable(parameter_, error);
  LOG_FINEST() << "value = " << (*value_);

  utilities::RandomNumberGenerator& rng = utilities::RandomNumberGenerator::Instance();

  Double new_value = rng.normal(AS_DOUBLE(mu_), AS_DOUBLE(sigma_));
  (*value_) = new_value;

  if ((*value_) < lower_bound_) {
    LOG_FINEST() << "hit @estimate lower bound setting value from " << (*value_) << " to " << lower_bound_;
    (*value_) = lower_bound_;
  }
  if ((*value_) > upper_bound_) {
    LOG_FINEST() << "hit @estimate upper bound setting value from " << (*value_) << " to " << upper_bound_;
    (*value_) = upper_bound_;
  }

  LOG_FINE() << "Setting Value to: " << (*value_);
  parameter_by_year_[model_->current_year()] = (*value_);
  (this->*update_function_)(*value_);
}

} /* namespace timevarying */
} /* namespace niwa */