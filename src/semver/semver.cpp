#include <iostream>
#include <getopt.h>
#include <unistd.h> // isatty()
#include <cstdio> // fileno()
#include <vector>
#include <libsemver/gettext.h>
#include <libsemver/gettext_defs.h>
#include "libsemver/c++/version.hpp"
#include "../../libsemver_config.h"

static bool command_set = false;
static bool cflag = false;
static bool vflag = false;

static void parse_opts(int argc, char **argv);
static std::vector<std::string> read_arguments(int argc, char **argv);
static void usage(std::ostream& stream);

static int check_versions(std::vector<std::string> vers);

int main(int argc, char **argv)
{
  // Trigger gettext operations
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  parse_opts(argc, argv);

  if (!command_set)
  {
    std::cerr << _("No command has been specified.\n");
    exit(1);
  }

  // Extract arguments from the command line and for the pipe.
  std::vector<std::string> args = read_arguments(argc, argv);

  // validate options
  if (args.size() == 0)
  {
    std::cerr << _("Invalid number of arguments.\n");
    exit(1);
  }

  if (vflag)
  {
    return check_versions(args);
  }

  return 0;
}

int check_versions(std::vector<std::string> vers)
{
  int ret = 0;

  for (auto v : vers)
  {
    try
    {
      semver::version::from_string(v);
    }
    catch (std::invalid_argument& ex)
    {
      std::cerr << ex.what() << "\n";
      ret = 1;
    }
  }

  return ret;
}

std::vector<std::string> read_arguments(int argc, char **argv)
{
  std::vector<std::string> arguments;

  for (auto i = optind; i < argc; ++i) arguments.push_back(argv[i]);

  // Read arguments from the standard input, if the program is not connected to
  // a terminal
  std::string pipe_input;
  if (!isatty(fileno(stdin)))
  {
    while (std::cin >> pipe_input)
    {
      arguments.push_back(pipe_input);
    }
  }

  return arguments;
}

void parse_opts(int argc, char **argv)
{
  int ch;
  std::string short_options = "chv";

  int option_index = 0;
  static struct option long_options[] = {
    {"compare", no_argument, nullptr, 'c'},
    {"help",    no_argument, nullptr, 'h'},
    {"valid",   no_argument, nullptr, 'v'},
    {nullptr, 0,             nullptr, 0}
  };

  while ((ch = getopt_long(argc,
                           argv,
                           short_options.c_str(),
                           long_options,
                           &option_index)) != -1)
  {
    switch (ch)
    {
    case 'c':
      command_set = true;
      cflag = true;
      break;

    case 'h':
      usage(std::cout);
      exit(0);

    case 'v':
      command_set = true;
      vflag = true;
      break;

    case '?':
      usage(std::cerr);
      exit(1);
    }
  }
}

void usage(std::ostream& stream)
{
  stream << PACKAGE_STRING << "\n\n";
  stream << _("Usage:\n");
  stream << "semver" << _(" [OPTION] ... path ...\n");
  stream << "\n";
  stream << _("Options:\n");
  stream << " -h, --help            " << _("Show this message.\n");
  stream << "\n";
  stream << _("See the man page for more information.\n\n");
  stream << _("Report bugs to <") << PACKAGE_BUGREPORT << ">.\n";
  stream << _("Project home page: <") << PACKAGE_URL << ">.";
  stream << "\n";
}
