#ifndef PARSER_H
#define PARSER_H

#include "../shared/fw_rule.h"

#define MAX_LINE 256
#define DEFAULT_CONFIG_PATH "config/biiid.conf"

struct fw_rule_node {
    struct fw_rule rule;
    struct fw_rule_node *next;
};

char *get_default_config_path(void);
struct fw_rule_node *parse_rules(const char *path);
void free_rule_list(struct fw_rule_node *head);
void print_rule_list(struct fw_rule_node *head);

#endif
