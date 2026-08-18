#include "watcher.h"
#include "parser.h"
#include "netlink_client.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <stdio.h>

int watch_config(const char *path){
  int fd = inotify_init();
  if(fd == -1){
    perror("inotify_init()");
    return 1;
  }
  int wd = inotify_add_watch(fd, path, IN_CLOSE_WRITE);
  if(wd == -1){
    perror("inotify_add_watch()\n");
    return 1;
  }

  while(1){
    char buff[4096];
    int len = read(fd, buff, sizeof(buff));
    if(len <= 0){
      perror("read()\n");
      return 1;
    }
    struct inotify_event *event;
    for(char *ptr = buff;
        ptr < buff + len; ptr += sizeof(struct inotify_event) + event->len){

      event = (struct inotify_event*)ptr;
      if(event->mask & IN_CLOSE_WRITE){
        printf("file modified, reparsing\n");

        struct fw_rule_node *rules = parse_rules(path);
        if (rules == NULL) {
          fprintf(stderr, "watch_config: reparse failed, keeping previous rules\n");
          continue;
        }

        send_rules(rules);
        free_rule_list(rules);
      }
    }
  }
  return 0;
}
