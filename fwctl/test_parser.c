#include "parser.h"
#include <stdio.h>

int main(void)
{
    char *path = get_default_config_path();
    if (!path) {
        fprintf(stderr, "Could not determine config path\n");
        return 1;
    }
    printf("Using config path: %s\n", path);

    struct fw_rule_node *rules = parse_rules(path);
    if (!rules) {
        fprintf(stderr, "No rules parsed (file missing, empty, or all lines invalid)\n");
        return 1;
    }

    print_rule_list(rules);

    free_rule_list(rules);
    return 0;
}
