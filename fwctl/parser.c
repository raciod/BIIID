#include "parser.h"
#include "../shared/fw_rule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_LINE 256

static __u8 parse_protocol(const char *s)
{
  if(strcmp(s, "TCP") == 0)  return IPPROTO_TCP;
  if(strcmp(s, "UDP") == 0)  return IPPROTO_UDP;
  if(strcmp(s, "ICMP") == 0) return IPPROTO_ICMP;
  return 0;
}

static __u8 parse_direction(const char *s)
{
  if(strcmp(s, "OUT") == 0) return FW_DIR_OUT;
  if(strcmp(s, "IN")  == 0) return FW_DIR_IN;
  return FW_DIR_IN;
}

static __u8 parse_action(const char *s)
{
  if (strcmp(s, "DROP") == 0)   return FW_ACTION_DROP;
  if (strcmp(s, "ACCEPT") == 0) return FW_ACTION_ACCEPT;
  return FW_ACTION_DROP;
}


char *get_default_config_path(void) {
    return DEFAULT_CONFIG_PATH;
}

// static int parse_line(const char )



struct fw_rule_node *parse_rules(const char *path){
  // Open the file 
  FILE *fd = fopen(path, "r");
  if(fd == NULL) {
    perror("error in fopen");
    return NULL;
  }

  int field_number;
  char line[MAX_LINE];
  char action[16], field[16], value[64], direction[16];
  
  struct fw_rule_node *head = NULL;
  struct fw_rule_node *tails = NULL;

  while(fgets(line, sizeof(line), fd) != 0){
  
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '#' || *p == '\n' || *p == '\0') continue;

    // copy the line to the node
    field_number = sscanf(line, "%15s %15s %63s %15s", action, field, value, direction);
    if(field_number != 4) {
      fprintf(stderr, "SYNTAX error in the config file");
      continue;
    }

    if (strcmp(action, "DROP") != 0 && strcmp(action, "ACCEPT") != 0) {
      fprintf(stderr, "UNKNOWN Action only 'DROP'/'ACCEPT' Allowed: %s\n", action);
      continue;
    }
 
    if (strcmp(direction, "IN") != 0 && strcmp(direction, "OUT") != 0) {
      fprintf(stderr, "UNKNOWN Direction only 'IN' and 'OUT' are Allowed: %s\n", direction);
      continue;
    }

    struct fw_rule_node *current_node = (struct fw_rule_node *)malloc(sizeof(struct fw_rule_node));
    if(!current_node){
      perror("malloc(current_node)");
      fclose(fd);
      free_rule_list(head);
      return NULL;
    }
    memset(current_node, 0, sizeof(struct fw_rule_node));
    current_node->next = NULL;
    current_node->rule.action = parse_action(action);
    current_node->rule.direction = parse_direction(direction);

    if (strcmp(field, "SRC_IP") == 0) {
      struct in_addr addr;
      if (inet_pton(AF_INET, value, &addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", value);
        free(current_node);
        continue;
      }
      current_node->rule.src_ip = addr.s_addr;
 
    } else if (strcmp(field, "DST_IP") == 0) {
      struct in_addr addr;
      if (inet_pton(AF_INET, value, &addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", value);
        free(current_node);
        continue;
      }
      current_node->rule.dst_ip = addr.s_addr;
 
    } else if (strcmp(field, "PORT") == 0) {
      current_node->rule.port = (__u16)atoi(value);
 
    } else if (strcmp(field, "PROTO") == 0) {
      current_node->rule.protocol = parse_protocol(value);
 
    } else {
      fprintf(stderr, "UNKNOWN field only 'SRC_IP', 'DST_IP', 'PORT', 'PROTO' Allowed: %s\n", field);
      free(current_node);
      continue;
    }
  
    // add it to the linked list
    if (head == NULL){
      head = current_node;
      tails = current_node;
    }else {
      tails->next = current_node;
      tails = current_node;
    }

  }
  fclose(fd);
  return head; 
}





/* for test only =============*/
static const char *protocol_name(__u8 proto)
{
  switch (proto) {
    case IPPROTO_TCP:  return "TCP";
    case IPPROTO_UDP:  return "UDP";
    case IPPROTO_ICMP: return "ICMP";
    case 0:            return "ANY";
    default:           return "UNKNOWN";
  }
}
 
void print_rule_list(struct fw_rule_node *head)
{
  int count = 0;
  char src_buf[INET_ADDRSTRLEN];
  char dst_buf[INET_ADDRSTRLEN];
 
  struct fw_rule_node *node = head;
  while (node) {
    struct fw_rule *r = &node->rule;
 
    struct in_addr src = { .s_addr = r->src_ip };
    struct in_addr dst = { .s_addr = r->dst_ip };
    inet_ntop(AF_INET, &src, src_buf, sizeof(src_buf));
    inet_ntop(AF_INET, &dst, dst_buf, sizeof(dst_buf));
 
    printf("Rule %d: action=%s src_ip=%s dst_ip=%s proto=%s port=%u direction=%s\n",
           count,
           r->action == FW_ACTION_DROP ? "DROP" : "ACCEPT",
           r->src_ip ? src_buf : "ANY",
           r->dst_ip ? dst_buf : "ANY",
           protocol_name(r->protocol),
           r->port,
           r->direction == FW_DIR_IN ? "IN" : "OUT");
 
    node = node->next;
    count++;
  }
 
  printf("Total rules: %d\n", count);
}

/* ===================== */

void free_rule_list(struct fw_rule_node *head){
  while(head){
    struct fw_rule_node *next = head->next;
    free(head);
    head = next;
  }
}



