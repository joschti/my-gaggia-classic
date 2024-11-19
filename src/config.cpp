#include "config.hpp"

namespace config
{
  float targetTemp = 87.0f;
  float ctrlHeaterProp = 1.0f;
  float ctrlHeaterInt = 0.25f;
  float ctrlHeaterDiff = 0.25f;
}

// https://stackoverflow.com/questions/10892167/proper-implementation-of-global-configuration
// bool config::load_config_file()
// {
//   // Code to load and set the configuration variables
// }