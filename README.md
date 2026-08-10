# Fire-Wall
 
A small Linux firewall built as a kernel module, using the Netfilter framework to inspect and filter packets.
 
## Build & run
 
**Requirements:** kernel headers matching your running kernel (`/usr/src/kernels/$(uname -r)` or equivalent).
 
**Compile:**
```
make
```
 
**Load the module:**
```
sudo insmod netfilter_module.ko
```
 
**Check the kernel log:**
```
sudo dmesg | tail
```
 
**Unload the module:**
```
sudo rmmod netfilter_module
```
 
aciod
