#include "parser.h"
#include "netlink_client.h"
#include "watcher.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int watch_mode = 0;
    const char *path = DEFAULT_CONFIG_PATH;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--watch") == 0 || strcmp(argv[i], "-w") == 0) {
            watch_mode = 1;
        } else if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    printf("Using config path: %s\n", path);

    struct fw_rule_node *rules = parse_rules(path);
    if (rules == NULL) {
        fprintf(stderr, "error in parse_rules\n");
        return 1;
    }

    int result = send_rules(rules);
    free_rule_list(rules);

    if (result != 0) {
        fprintf(stderr, "send_rules failed\n");
        return 1;
    }

    // Enter watch loop if requested
    if (watch_mode) {
        printf("Entering watch mode on %s...\n", path);
        return watch_config(path);
    }

    return 0;
}
