
#include "rexgenparsercontext.h"
#include "Regex.h"
#include "librexgen.h"
#include <cstdio>
#include <execinfo.h>
#include <signal.h>
#include "unicode.h"
#include <uniconv.h>
#include <locale.h>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

using namespace std;
#ifdef YYDEBUG
extern int rexgen_debug;
#endif
static void usage() {
  cerr << "Usage: rexgen <regex>" << endl;
  cerr << "Locale: " << locale_charset() << endl;
}


static void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

static void initLogger() {
  std::string initFileName = "log4cpp.properties";
  log4cpp::PropertyConfigurator::configure(initFileName);
}

static void setlocale() {
  const char* defaultLocale = "en_US.UTF8";
  const char* sysLocale = NULL;
  
  if ((sysLocale = getenv("LC_CTYPE")) != NULL) {
    setlocale(LC_CTYPE, sysLocale);
  }
  if ((sysLocale = getenv("LC_MESSAGES")) != NULL) {
    setlocale(LC_CTYPE, sysLocale);
  }
  if ((sysLocale = getenv("LC_ALL")) != NULL) {
    setlocale(LC_CTYPE, sysLocale);
  }
  if (sysLocale == NULL) {
    setlocale(LC_ALL, defaultLocale);
  }
}

int main(int argc, char** argv) {
  char_type xml[1024];
  char_type buffer[512];
  const char* format;
  int len;
#ifdef YYDEBUG
  rexgen_debug=1;
#endif
  
  setlocale();
  initLogger();
  
  if (argc != 2) {
    usage();
    exit(1);
  }

  signal(SIGSEGV, handler);
  signal(SIGABRT, handler);
  
  cout << "parsing '" << argv[1] << "'" << endl;
  Regex* regex = parse_regex(argv[1]);
  
  if (regex != NULL) {
    regex->appendRawValue(xml, sizeof(xml)/sizeof(xml[0]));
#if defined(UTF8)
    format = "result:\n%U\n";
#elif defined(UTF16)
    format = "result:\n%lU\n";
#else
    format = "result:\n%llU\n";
#endif
    ulc_fprintf(stdout, format, xml);

    cout<< "length: " << regex->getMaxSize() << endl;
    
    Iterator* iter = regex->iterator();

#if defined(UTF8)
    format = "value: %U (length: %d)\n";
#elif defined(UTF16)
    format = "value: %lU (length: %d)\n";
#else
    format = "value: %llU (length: %d)\n";
#endif
    while (iter->hasNext()) {
	iter->next();
	len = iter->value(buffer, sizeof(buffer)/sizeof(buffer[0])-1);
	buffer[len] = '\0';
	
	ulc_fprintf(stdout, format, buffer, len);
    }
    
    delete regex;
  }
  log4cpp::Category::shutdown();
  return 0;
}
