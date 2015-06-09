#ifdef TESTMODE
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#else

// defines
#ifndef BOOST_USE_WINDOWS_H
#define BOOST_USE_WINDOWS_H
#endif

// Headers
#include <iostream>
#include <boost/thread.hpp>

#include "Version.h"
#include "ConfigurationLoader/Loader.h"
#include "GlobalConfiguration/GlobalConfiguration.h"
#include "Model/Model.h"
#include "Reports/Children/StandardHeader.h"
#include "Reports/Manager.h"
#include "Utilities/CommandLineParser/CommandLineParser.h"
#include "Logging/Logging.h"

// Namespaces
using namespace niwa;
using std::cout;
using std::endl;

// local variables
RunMode::Type run_mode = RunMode::kInvalid;
bool model_start_return_success = true;

void ModelThread() {
  // Run the model
  ModelPtr model = Model::Instance();
  model_start_return_success = model->Start(run_mode);

  reports::Manager::Instance().set_continue(false);
}

/**
 *
 */
void ReportThread() {
  reports::Manager::Instance().FlushReports();
}

/**
 * Application entry point
 */
int main(int argc, char * argv[]) {
  int return_code = 0;
  // Create instance now so it can record the time.
  reports::StandardHeader standard_report;

  /**
   * Store our command line parameters
   */
  GlobalConfigurationPtr config = GlobalConfiguration::Instance();

  vector<string> parameters;
  for (int i = 0; i < argc; ++i)
    parameters.push_back(argv[i]);
  config->set_command_line_parameters(parameters);

  try {
    /**
     * Ask the runtime controller to parse the parameters.
     */
    niwa::utilities::CommandLineParser parser;
    parser.Parse(argc, (const char **)argv);

    run_mode = parser.run_mode();

    /**
     * Check the run mode and call the handler.
     */
    switch (run_mode) {
    case RunMode::kInvalid:
      LOG_ERROR() << "Invalid run mode specified.";
      break;

    case RunMode::kVersion:
      cout << SOURCE_CONTROL_VERSION << endl;
      break;

    case RunMode::kLicense:
      cout << "License has not been implemented yet" << endl;
      break;

    case RunMode::kHelp:
      cout << parser.command_line_usage() << endl;
      break;

    case RunMode::kQuery:
      break;

    default:
      if (!config->debug_mode() && !config->disable_standard_report())
        standard_report.Prepare();

      /**
       * Load our configuration files
       */
      configuration::Loader config_loader;
      config_loader.LoadConfigFile();
      if (Logging::Instance().errors().size() > 0) {
        Logging::Instance().FlushErrors();
        return_code = -1;
        break;
      }

      /**
       * Override any config values
       */
      config->OverrideGlobalValues(parser.override_values());

      /**
       * build our threads
       */
      boost::thread model_thread(ModelThread);
      boost::thread report_thread(ReportThread);

      model_thread.join();
      report_thread.join();

      Logging& logging = Logging::Instance();
      if (logging.errors().size() > 0) {
        logging.FlushErrors();
        return_code = -1;
      }

      logging.FlushWarnings();

      if (!config->debug_mode() && !config->disable_standard_report())
        standard_report.Finalise();
      break;
    }

  } catch (const string &isam_exception) {
    /**
     * This is the standard method of printing an error to the user. So
     * we expect exceptions to be thrown up cleanly.
     */
    cout << "## ERROR - iSAM experienced a problem and has stopped execution" << endl;
    cout << "## Execution stack: " << endl;

    // Un-Wind our Exception Stack
    int last_location = 0;
    while (last_location != -1) {
      int location = isam_exception.find_first_of(">", last_location+1);
      cout << isam_exception.substr(last_location+1, (location-last_location)) << endl;
      last_location = location;
    }
    cout << endl;
    return -1;

  } catch (std::exception& e) {
    cout << "## ERROR - iSAM experienced a problem and has stopped execution" << endl;
    cout << e.what() << endl;

  } catch(...) {
    cout << "## ERROR - iSAM experienced a problem and has stopped execution" << endl;
  }

  if (!model_start_return_success) {
    LOG_FINEST() << "Done with errors";
    return -1;
  }

  LOG_FINEST() << "Done";
	return return_code;
}
#endif


