# BIIID

A custom Linux kernel firewall built with the Netfilter framework and controlled via a user-space CLI tool (`fwctl`) over Netlink sockets.

[English](#english) | [Français](#français)

---

## English

### Overview
BIIID is a proof-of-concept Linux firewall written entirely in C. It separates packet filtering logic (executed inside kernel space via Netfilter hooks) from rule management (handled in user space via a CLI). Rules defined in a configuration file are sent directly to the kernel via Netlink sockets without requiring module re-compilation or reload.

> **Warning:** This tool intercepts traffic at `LOCAL_IN` and `LOCAL_OUT`. Test strictly inside a virtual machine to avoid losing access to your host machine or network session.

### Key Features
* **Kernel-Level Filtering:** Hooks into `NF_INET_LOCAL_IN` and `NF_INET_LOCAL_OUT` for network packet evaluation.
* **Netlink IPC:** Transfers binary rule structures (`struct fw_rule`) from user space directly to the kernel list via socket option 31 (`NETLINK_USER`).
* **Zero Kernel Overhead Parsing:** Config parsing happens in user space (`fwctl`), keeping kernel memory operations minimal and fast.
* **Dynamic Linked List Storage:** Evaluates multiple active filtering rules stored using `linux/list.h`.

### Requirements
* Linux system with matching kernel headers installed
* GCC and Make
* Root/`sudo` privileges for kernel module loading

### Build & Usage

```bash
# Build the kernel module and fwctl CLI
make

# Load the kernel module
sudo make install

# Push configuration rules from biiid.conf to kernel space
./fwctl/fwctl

# View firewall drop/match logs
sudo make log

# Unload the kernel module
sudo make remove

```

---

## Français

### Aperçu

BIIID est un pare-feu expérimental pour Linux entièrement développé en C. Il sépare l'évaluation du trafic (exécutée dans le noyau via les hooks Netfilter) de la gestion des règles (exécutée dans l'espace utilisateur via un outil CLI). Les règles du fichier de configuration sont transmises au noyau via des sockets Netlink sans nécessiter la recompilation du module.

> **Attention :** Cet outil intercepte le trafic sur `LOCAL_IN` et `LOCAL_OUT`. Il est fortement recommandé de le tester dans une machine virtuelle pour éviter toute coupure de réseau accidentelle.

### Caractéristiques

* **Filtrage au niveau du Noyau :** S'attache aux hooks `NF_INET_LOCAL_IN` et `NF_INET_LOCAL_OUT` pour évaluer les paquets.
* **Communication IPC Netlink :** Transmet directement des structures binaires (`struct fw_rule`) vers la liste du noyau via `NETLINK_USER`.
* **Traitements Légers dans le Noyau :** L'analyse des fichiers de configuration est faite par la CLI (`fwctl`), évitant tout parsing texte lourd au niveau du noyau.
* **Stockage Dynamique :** Stocke et évalue plusieurs règles simultanées à l'aide des listes chaînées du noyau (`linux/list.h`).

### Prérequis

* Système Linux avec les en-têtes du noyau (*kernel headers*) installés
* GCC et Make
* Droits Root / `sudo` pour charger le module noyau

### Compilation & Utilisation

```bash
# Compiler le module noyau et la CLI fwctl
make

# Charger le module dans le noyau
sudo make install

# Appliquer les règles du fichier biiid.conf au noyau
./fwctl/fwctl

# Consulter les logs du pare-feu
sudo make log

# Décharger le module du noyau
sudo make remove

```

---

## Rule Syntax / Syntaxe des Règles (`biiid.conf`)

Rules are defined line-by-line in `config/biiid.conf`:

```text
# Action     Field         Value               Direction
DROP         SRC_IP        192.168.1.100       IN
DROP         DST_IP        192.168.1.100       OUT
DROP         PORT          80                  OUT
DROP         PROTO         TCP                 IN

```

```

```
