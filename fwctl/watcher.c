#include "watcher.h"
#include "parser.h"
#include "netlink_client.h"
#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int watch_config(const char *path)
{
    int inotify_fd = inotify_init();
    if (inotify_fd == -1) {
        perror("inotify_init");
        return 1;
    }

    int wd = inotify_add_watch(inotify_fd, path, IN_CLOSE_WRITE);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(inotify_fd);
        return 1;
    }

    int nl_fd = init_netlink_socket();
    if (nl_fd < 0) {
        fprintf(stderr, "watch_config: failed to initialize netlink socket\n");
        close(inotify_fd);
        return 1;
    }

    struct fw_rule_node *initial_rules = parse_rules(path);
    if (initial_rules) {
        send_rules_fd(nl_fd, initial_rules);
        free_rule_list(initial_rules);
    }

    struct pollfd fds[2];
    
    fds[0].fd = inotify_fd;
    fds[0].events = POLLIN;

    fds[1].fd = nl_fd;
    fds[1].events = POLLIN;

    printf("[WATCHER] Listening for config changes and kernel drop events...\n");

    while (1) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            char buff[4096];
            int len = read(inotify_fd, buff, sizeof(buff));
            if (len > 0) {
                struct inotify_event *event;
                for (char *ptr = buff; ptr < buff + len; ptr += sizeof(struct inotify_event) + event->len) {
                    event = (struct inotify_event *)ptr;
                    if (event->mask & IN_CLOSE_WRITE) {
                        printf("\n[WATCHER] Config file modified, reparsing...\n");
                        struct fw_rule_node *rules = parse_rules(path);
                        if (rules == NULL) {
                            fprintf(stderr, "[WATCHER] Reparse failed, keeping previous rules\n");
                            continue;
                        }
                        send_rules_fd(nl_fd, rules);
                        free_rule_list(rules);
                    }
                }
            }
        }

        if (fds[1].revents & POLLIN) {
            receive_drop_notification(nl_fd);
        }
    }

    close(nl_fd);
    close(inotify_fd);
    return 0;
}
