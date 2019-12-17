#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cli_parse.h"

static const char short_opts[] = "r:d:m:t:f:v:";

static const struct option long_opts[] = {
    { "reference",        1, NULL, 'r' },
    { "distorted",        1, NULL, 'd' },
    { "model",            1, NULL, 'm' },
    { "threads",          1, NULL, 't' },
    { "feature",          1, NULL, 'f' },
    { "version",          0, NULL, 'v' },
    { NULL,               0, NULL, 0 },
};

static void usage(const char *const app, const char *const reason, ...) {
    if (reason) {
        va_list args;
        va_start(args, reason);
        vfprintf(stderr, reason, args);
        va_end(args);
        fprintf(stderr, "\n\n");
    }
    fprintf(stderr, "Usage: %s [options]\n\n", app);
    fprintf(stderr, "Supported options:\n"
            " --reference/-r $string:    path to reference .y4m\n"
            " --distorted/-d $string:    path to distorted .y4m\n"
            " --model/-m $model:         path to model file\n"
            " --threads/-t $unsigned:    number of threads to use\n"
            " --feature/-f $string:      additional feature\n"
            " --version/-v:              print version and exit\n"
           );
    exit(1);
}

static void error(const char *const app, const char *const optarg,
                  const int option, const char *const shouldbe)
{
    char optname[256];
    int n;

    for (n = 0; long_opts[n].name; n++)
        if (long_opts[n].val == option)
            break;
    assert(long_opts[n].name);
    if (long_opts[n].val < 256) {
        sprintf(optname, "-%c/--%s", long_opts[n].val, long_opts[n].name);
    } else {
        sprintf(optname, "--%s", long_opts[n].name);
    }

    usage(app, "Invalid argument \"%s\" for option %s; should be %s",
          optarg, optname, shouldbe);
}

static unsigned parse_unsigned(const char *const optarg, const int option,
                               const char *const app)
{
    char *end;
    const unsigned res = (unsigned) strtoul(optarg, &end, 0);
    if (*end || end == optarg) error(app, optarg, option, "an integer");
    return res;
}

void cli_parse(const int argc, char *const *const argv,
               CLISettings *const settings)
{
    memset(settings, 0, sizeof(*settings));
    int o;

    while ((o = getopt_long(argc, argv, short_opts, long_opts, NULL)) >= 0) {
        switch (o) {
        case 'r':
            settings->y4m_path_ref = optarg;
            break;
        case 'd':
            settings->y4m_path_dist = optarg;
            break;
        case 'm':
            if (settings->model_cnt == CLI_SETTINGS_ARRAY_LEN) {
                usage(argv[0], "A maximum of %d models is supported\n",
                      CLI_SETTINGS_ARRAY_LEN);
            }
            settings->model_path[settings->model_cnt++] = optarg;
            break;
        case 'f':
            if (settings->feature_cnt == CLI_SETTINGS_ARRAY_LEN) {
                usage(argv[0], "A maximum of %d features is supported\n",
                      CLI_SETTINGS_ARRAY_LEN);
            }
            settings->feature[settings->feature_cnt++] = optarg;
            break;
        case 't':
            settings->thread_cnt = parse_unsigned(optarg, 't', argv[0]);
            break;
        default:
            break;
        }
    }

    if (!settings->y4m_path_ref)
        usage(argv[0], "Reference .y4m (-r/--reference) is required");
    if (!settings->y4m_path_ref)
        usage(argv[0], "Distorted .y4m (-d/--distorted) is required");
    if (settings->model_cnt == 0)
        usage(argv[0], "At least one model file (-m/--model) is required");
}
